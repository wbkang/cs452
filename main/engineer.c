#include <engineer.h>
#include <syscall.h>
#include <server/traincmdbuffer.h>
#include <server/traincmdrunner.h>
#include <fixed.h>
#include <track_data.h>
#include <stop_distance.h>
#include <betaimporter.h>
#include <uconst.h>

engineer *engineer_new(char track_name) {
	engineer *this = malloc(sizeof(engineer));

	// initialize helper tasks
	this->tid_traincmdbuf = traincmdbuffer_new();
	traincmdrunner_new();

	// start
	train_go(this->tid_traincmdbuf);

	// initialize train descriptors
	TRAIN_FOREACH(train_no) {
		train_descriptor *train = &this->train[train_no];
		train->no = train_no;
		train->stopm = fixed_new(0);
		train->stopb = fixed_new(0);
		train->speed = 0;
		train->last_speed = 0;
		train->dir = TRAIN_UNKNOWN;
		location_init_undef(&train->loc);
		train->v = fixed_new(0);
		train->last_sensor = NULL;
		train->timestamp_last_sensor = 0;
		train->timestamp_last_spdcmd = 0;
		train->timestamp_last_nudged = 0;
		TRAIN_FOREACH_SPEEDIDX(speed) {
			train->tref[speed] = -1;
			train->dref = -1;
		}
		populate_stop_distance(&this->train[train_no], train_no);
	}

	// initialize track nodes
	track_node *tn = malloc(sizeof(track_node) * TRACK_MAX);
	switch (track_name) {
		case 'a':
			this->track_nodes = init_tracka(tn);
			// @TODO: instead of using track b betas, get track a betas
			populate_beta(this->track_nodes);
			break;
		case 'b':
			this->track_nodes = init_trackb(tn);
			populate_beta(this->track_nodes);
			break;
		default:
			ERROR("bad track name");
			break;
	}

	this->con = console_new(WhoIs(NAME_IOSERVER_COM2));
	this->log = logdisplay_new(this->con, 24, 55, 10, ROUNDROBIN);
	this->trainloc = logstrip_new(this->con, 35, 55);
	this->tid_time = WhoIs(NAME_TIMESERVER);

	return this;
}

void engineer_set_tref(engineer *this, int train_no, int speed_idx, int tref) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	ASSERT(0 <= speed_idx && speed_idx < TRAIN_NUM_SPEED_IDX, "bad speed_idx");
	this->train[train_no].tref[speed_idx] = tref;
}

// @TODO: remove speed_idx. return the tref for the current speed
int engineer_get_tref(engineer *this, int train_no, int speed_idx) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	ASSERT(0 <= speed_idx && speed_idx < TRAIN_NUM_SPEED_IDX, "bad speed_idx");
	return this->train[train_no].tref[speed_idx];
}

int engineer_get_dref(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	return this->train[train_no].dref;
}
void engineer_set_dref(engineer *this, int train_no, int dref) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	this->train[train_no].dref = dref;
}

void engineer_set_stopinfo(engineer *this, int train_no, fixed m, fixed b) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	train->stopm = m;
	train->stopb = b;
}

void engineer_get_stopinfo(engineer *this, int train_no, fixed *m, fixed *b) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	*m = train->stopm;
	*b = train->stopb;
}

// this function simulates the distance that the train would travel if it were to stop immediately
// @TODO: gather the stopping distance in terms of velocity to make this code more robust
fixed engineer_sim_stopdist(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	return fixed_add(fixed_mul(train->stopm, fixed_new(train->speed)), train->stopb);
}

// @TODO: improve "speed & last_speed" to include last N timestamped speed changes and use these to improve "engineer_train_move"
static void engineer_on_set_speed(engineer *this, int train_no, int speed) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	train->last_speed = train->speed;
	train->speed = speed;
	train->v = fixed_new(-1); // invalidate velocity
	train->timestamp_last_spdcmd = Time(this->tid_time);
}

void engineer_set_speed(engineer *this, int train_no, int speed) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	if (engineer_get_speed(this, train_no) == speed) return;
	train_speed(train_no, speed, this->tid_traincmdbuf);
	// @TODO: there is a delay between putting the bytes in UART and when the train is aware of them. we need to include this delay right here. we could use a blocking putc and a command runner that pings the engineer back saying the command was put into the UART. we delay the following line until then.
	engineer_on_set_speed(this, train_no, speed);
}

int engineer_get_speed(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	return this->train[train_no].speed;
}

fixed engineer_get_velocity(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	return this->train[train_no].v;
}

void engineer_get_loc(engineer *this, int train_no, location *loc) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	*loc = this->train[train_no].loc;
}

void engineer_reverse(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	int speed = engineer_get_speed(this, train_no);
	fixed dstop = engineer_sim_stopdist(this, train_no);

	fixed v = engineer_get_velocity(this, train_no);
	engineer_set_speed(this, train_no, 0);
	if (fixed_sgn(v) > 0) {
		fixed stoptime = fixed_div(fixed_mul(fixed_new(2), dstop), v);
		stoptime = fixed_add(stoptime, fixed_new(MS2TICK(500)));
		traincmdbuffer_put(this->tid_traincmdbuf, PAUSE, fixed_int(stoptime), NULL);
		traincmdbuffer_put(this->tid_traincmdbuf, REVERSE, train_no, NULL);
		traincmdbuffer_put(this->tid_traincmdbuf, PAUSE, TRAIN_PAUSE_AFTER_REVERSE, NULL);
	} else {
		train_reverse(train_no, this->tid_traincmdbuf); // regular safe pause
	}
	engineer_train_set_dir(this, train_no, train_opposite_direction(engineer_train_get_dir(this, train_no)));
	engineer_set_speed(this, train_no, speed);
}

int engineer_get_speedidx(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	return train_speed2speed_idx(&this->train[train_no]);
}

void engineer_train_pause(engineer *this, int train_no, int ticks) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	// @TODO: implement a way to pause trains independently
	(void) train_no;
	if (ticks > 0) {
		traincmdbuffer_put(this->tid_traincmdbuf, PAUSE, ticks, NULL);
	}
}

void engineer_set_track(engineer *this, int s[], int ns, int c[], int nc) {
	while (ns--) {
		engineer_set_switch(this, *s++, 's', FALSE);
	}
	while (nc--) {
		engineer_set_switch(this, *c++, 'c', FALSE);
	}
	train_solenoidoff(this->tid_traincmdbuf);
}

track_node *engineer_get_tracknode(engineer *this, char *type, int id) {
	char name[8];
	sprintf(name, "%s%d", type, id);
	track_node *rv = NULL;
	if (lookup_goodkey(this->track_nodes, name)) {
		rv = lookup_get(this->track_nodes, name);
	}
	ASSERT(rv, "rv is null, type: %s, id: %d", type, id);
	return rv;
}

void engineer_set_switch(engineer *this, int id, int pos, int offsolenoid) {
	track_node *br = engineer_get_tracknode(this, "BR", id);
	int dir = POS2DIR(pos);
	if (br->switch_dir != dir) { // assume only engineer switches branches
		br->switch_dir = dir;
		train_switch(id, pos, this->tid_traincmdbuf);
	}
	if (offsolenoid) { // might be the last switch in a series
		train_solenoidoff(this->tid_traincmdbuf);
	}
}

void engineer_destroy(engineer *this) {
	TRAIN_FOREACH(train_no) {
		if (engineer_get_speed(this, train_no) > 0) {
			engineer_set_speed(this, train_no, 0);
		}
	}
	train_stop(this->tid_traincmdbuf);
	Flush(WhoIs(NAME_IOSERVER_COM1));
}

train_direction engineer_train_get_dir(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	return this->train[train_no].dir;
}

void engineer_train_set_dir(engineer *this, int train_no, train_direction dir) {
	this->train[train_no].dir = dir;
}

// @TODO: train->v is the average velocity between the last and current sensor
//			perhaps here it should be set to the estimated/stored average velocity
//			between the current and the next sensor? or at least the total average
//			velocity for the entire track.
static void engineer_train_onsensor(engineer *this, train_descriptor *train, track_node *sensor, int timestamp) {
	engineer_ontick(this); // update all states
	track_node *last_sensor = train->last_sensor;
	train->last_sensor = sensor;
	int timestamp_last_sensor = train->timestamp_last_sensor;
	train->timestamp_last_sensor = timestamp;

	if (train->timestamp_last_spdcmd > timestamp_last_sensor - MS2TICK(4000)) return; // let train settle

	int dt = TICK2MS(timestamp - timestamp_last_sensor);
	if (dt <= 0) return; // too soon?

	int dx = track_distance(last_sensor, sensor);
	if (dx <= 0) return; // too fast?

	fixed new_v = fixed_div(fixed_new(dx), fixed_new(dt));

	int now = Time(this->tid_time);
	fixed dt_sensorlag = fixed_new(TICK2MS(now - timestamp));
	fixed dx_sensorlag = fixed_mul(new_v, dt_sensorlag);
	location new_loc;
	location_init(&new_loc, sensor->edge, dx_sensorlag);

	if (!location_isundef(&train->loc) && fixed_sgn(train->v) > 0) {
		fixed dist = location_dist(&train->loc, &new_loc);
		if (fixed_sgn(dist) > 0) {

			// tmp
			fixed errfreeoff = fixed_mul(train->v, fixed_new(dt));
			location errfreeloc;
			location_init(&errfreeloc, last_sensor->edge, errfreeoff);
			location_inc(&errfreeloc, dx_sensorlag);
			fixed errfreedist = location_dist(&errfreeloc, &new_loc);
			//tmp

			logdisplay_printf(this->log,
				"[%7d] %s+%Fmm [%s+%Fmm] ~ %s+%Fmm (%Fmm/ms ~ %Fmm/ms) %Fmm [%Fmm]",
				TICK2MS(timestamp),
				train->loc.edge->src->name,
				train->loc.offset,
				errfreeloc.edge->src->name,
				errfreeloc.offset,
				new_loc.edge->src->name,
				new_loc.offset,
				train->v,
				new_v,
				dist,
				errfreedist
			);
			logdisplay_flushline(this->log);
		}
	}

	train->loc = new_loc;
	train->v = new_v;
	train->timestamp_last_nudged = now;
}

// @TODO: improve this by using train location & trajectory
// @TODO: if a train's location is unknown but it is the only one in motion, attribute the sensor to it (calibration)
static train_descriptor *engineer_attribute_sensor(engineer *this, track_node *sensor, int timestamp) {
	TRAIN_FOREACH(train_no) {
		train_descriptor *train = &this->train[train_no];
		if (train->speed > 0) return train;
	}
	logdisplay_printf(this->log, "spurious sensor %s", sensor->name);
	logdisplay_flushline(this->log);
	return NULL;
}

// @TODO: also use sensor OFF as it is just as accurate
void engineer_onsensor(engineer *this, char data[]) {
	msg_sensor *msg = (msg_sensor*) data;
	if (msg->state == OFF) return; // ignore sensor-off for now

	track_node *sensor = engineer_get_tracknode(this, msg->module, msg->id);
	ASSERTNOTNULL(sensor);

	train_descriptor *train = engineer_attribute_sensor(this, sensor, msg->timestamp);
	if (!train) return; // spurious sensor, ignore

	engineer_train_onsensor(this, train, sensor, msg->timestamp);
}

// @TODO: add acceleration
// @TODO: add jerk
// @TODO: use track info
// @TODO: this is very sensitive to small dt and small v, fix this somehow?
static void engineer_train_move(engineer *this, train_descriptor *train, int t_i, int t_f) {
	location *loc = &train->loc;
	if (location_isundef(loc)) return; // lost
	if (fixed_sgn(train->v) <= 0) return; // bad trajectory
	fixed dt = fixed_new(TICK2MS(t_f - t_i));
	fixed dx = fixed_mul(train->v, dt);
	location_inc(loc, dx);
	// print
	char *direction_str;
	switch (train->dir) {
		case TRAIN_FORWARD:
			direction_str = "forward";
			break;
		case TRAIN_BACKWARD:
			direction_str = "backward";
			break;
		default:
			direction_str = "unknown";
			break;
	}
	logstrip_printf(this->trainloc,
		"%-5s + %Fcm (%s)",
		loc->edge->src->name,
		fixed_div(loc->offset, fixed_new(10)),
		direction_str
	);
}

void engineer_ontick(engineer *this) {
	int timestamp = Time(this->tid_time);
	TRAIN_FOREACH(train_no) {
		train_descriptor *train = &this->train[train_no];
		engineer_train_move(this, train, train->timestamp_last_nudged, timestamp);
		train->timestamp_last_nudged = timestamp;
	}
}
