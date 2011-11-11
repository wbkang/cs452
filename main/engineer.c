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
		train->dir = TRAIN_FORWARD;
		train->loc_edge = NULL;
		train->loc_offset = fixed_new(0);
		train->v = fixed_new(0);
		train->last_sensor = NULL;
		train->timestamp_last_sensor = 0;
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
			// @TODO: instead of populating this with track b betas, get track a betas
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
	this->log = logdisplay_new(this->con, 24, 65, 10, SCROLLING);
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
// @TODO: use velocity instead
fixed engineer_sim_stopdist(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	fixed rv = fixed_add(fixed_mul(train->stopm, fixed_new(train->speed)), train->stopb);
	if (fixed_sign(rv) < 0) return fixed_new(0);
	return rv;
}

void engineer_set_speed(engineer *this, int train_no, int speed) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	train->last_speed = train->speed;
	train->speed = speed;
	train->timestamp_last_spdcmd = Time(this->tid_time);
	train_speed(train_no, speed, this->tid_traincmdbuf);
}

int engineer_get_speed(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	return this->train[train_no].speed;
}

void engineer_reverse(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	int speed = engineer_get_speed(this, train_no);
	fixed dstop = engineer_sim_stopdist(this, train_no);
	fixed dref = fixed_new(engineer_get_dref(this, train_no));
	int speed_idx = engineer_get_speedidx(this, train_no);
	fixed tref = fixed_new(engineer_get_tref(this, train_no, speed_idx));

	engineer_set_speed(this, train_no, 0);
	if (fixed_sign(dstop) > 0 && fixed_sign(dref) > 0 && fixed_sign(tref) > 0) {
		fixed v = fixed_div(dref, tref); // approximate v_seg
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

void engineer_pause_train(engineer *this, int train_no, int ticks) {
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
	ASSERTNOTNULL(rv);
	return rv;
}

void engineer_set_switch(engineer *this, int id, int pos, int offsolenoid) {
	track_node *br = engineer_get_tracknode(this, "BR", id);
	ASSERTNOTNULL(br);
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

static train_descriptor *engineer_attribute_sensor(engineer *this, track_node *sensor) {
	TRAIN_FOREACH(train_no) {
		train_descriptor *train = &this->train[train_no];
		if (train->speed != 0) return train;
	}
	return NULL;
}

// static fixed engineer_estimate_dt(engineer *this, int train_no, track_node *from, track_node *to) {
// 	fixed beta = beta_sum(from, to);
// 	if (fixed_sign(beta) < 0) return fixed_new(-1); // bad beta
// 	int speed_idx = engineer_get_speedidx(this, train_no);
// 	if (speed_idx < 0) return fixed_new(-2); // bad speed idx
// 	int tref = engineer_get_tref(this, train_no, speed_idx);
// 	return fixed_mul(beta, fixed_new(tref));
// }

void engineer_onsensor(engineer *this, char data[]) {
	msg_sensor *msg = (msg_sensor*) data;
	if (msg->state == OFF) return; // ignore off for now
	track_node *sensor = engineer_get_tracknode(this, msg->module, msg->id);
	train_descriptor *train = engineer_attribute_sensor(this, sensor);
	if (!train) return; // spurious sensor, ignore
	if (train->timestamp_last_spdcmd > Time(this->tid_time) - MS2TICK(4000)) {
		// let the velocity settle
		train->v = fixed_new(-1);
		return;
	}
	int timestamp = msg->timestamp;
	int timestamp_last = train->timestamp_last_sensor;
	train->timestamp_last_sensor = timestamp;
	int dt = TICK2MS(timestamp - timestamp_last);

	track_node *last_sensor = train->last_sensor;
	train->last_sensor = sensor;

	// fixed dt_est = engineer_estimate_dt(this, train->no, last_sensor, sensor);
	// fixed fdt = fixed_new(dt);
	// const fixed max_err = fixed_div(fixed_new(2), fixed_new(10));
	// fixed rat = fixed_abs(fixed_div(dt_est, fdt));
	// if (fixed_comp(rat, max_err) > 0) return; // out of bounds

	int dx = track_distance(last_sensor, sensor);
	fixed v = fixed_div(fixed_new(dx), fixed_new(dt));

	train->loc_edge = &sensor->edge[0];
	train->loc_offset = fixed_mul(v, fixed_new(Time(this->tid_time) - timestamp));
	train->v = v;

	logdisplay_printf(this->log,
		"[%7d] @%s + %F, %d/%d (%F)",
		TICK2MS(timestamp),
		sensor->name,
		train->loc_offset,
		dx,
		dt,
		train->v
	);
	logdisplay_flushline(this->log);
	// work
	// msg->timestamp
}

// @TODO: think this out. Would be nice to know current velocity
// void engineer_set_velocity(engineer *this, int train_no, fixed measured_v, fixed predicted_v) {
// 	if (measured_v == 0) return;
// 	if (abs(fixed_div(measure_v, predicted_v)) <= fixed_div(fixed_new(5), fixed_new(100)))
// 	if (fixed_div())
// 	this->valocity
// }
