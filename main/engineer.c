#include <engineer.h>
#include <syscall.h>
#include <server/traincmdbuffer.h>
#include <server/traincmdrunner.h>
#include <fixed.h>
#include <track_data.h>
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
		train_init(&this->train[train_no], train_no);
	}

	// initialize track nodes
	track_node *tn = malloc(sizeof(track_node) * TRACK_MAX);
	switch (track_name) {
		case 'a':
			this->track_nodes = init_tracka(tn);
			break;
		case 'b':
			this->track_nodes = init_trackb(tn);
			break;
		default:
			ASSERT(0, "bad track name");
			break;
	}

	this->con = console_new(COM2);
	this->log = logdisplay_new(this->con, 11, 56, 8, 40, ROUNDROBIN);
	this->log2 = logdisplay_new(this->con, 11 + 9, 56, 8, 40, ROUNDROBIN);
	this->tid_time = WhoIs(NAME_TIMESERVER);

	return this;
}

void engineer_destroy(engineer *this) {
	TRAIN_FOREACH(train_no) {
		train_descriptor *train = &this->train[train_no];
		if (train_get_speed(train) > 0) {
			engineer_set_speed(this, train_no, 0);
		}
	}
	train_stop(this->tid_traincmdbuf);
	Flush(WhoIs(NAME_IOSERVER_COM1));
}

fixed engineer_sim_stopdist(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	return train_get_stopdist(&this->train[train_no]);
}

// @TODO: the only external thing that changes a train state is the set_speed command. we should keep a history of these commands if we want to be able to rewind the simulation.
void engineer_on_set_speed(engineer *this, int train_no, int speed, int t) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	train_simulate(train, t);
	train_on_set_speed(train, speed, t);
}

void engineer_set_speed(engineer *this, int train_no, int speed) {
	*get_globalint() = uptime();
	train_speed(train_no, speed, this->tid_traincmdbuf);
	// @TODO: there is a delay between putting the bytes in UART and when the train is aware of them. we need to include this delay right here. we could use a blocking putc and a command runner that pings the engineer back saying the command was put into the UART. we delay the following line until then.
	engineer_on_set_speed(this, train_no, speed, Time(this->tid_time));
}

void engineer_get_loc(engineer *this, int train_no, location *loc) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	train_get_loc(train, loc);
}

void engineer_reverse(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];

	int speed = train_get_speed(train);
	fixed dstop = train_get_stopdist(train);

	fixed v = train_get_velocity(train);
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
	train_reverse_dir(train);
	engineer_set_speed(this, train_no, speed);
}

// @TODO: implement a way to pause trains/track independently
void engineer_train_pause(engineer *this, int train_no, int ticks) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
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

train_direction engineer_train_get_dir(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	return train_get_dir(train);
}

void engineer_train_set_dir(engineer *this, int train_no, train_direction dir) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	train_set_dir(train, dir);
}

void engineer_train_on_loc(engineer *this, train_descriptor *train, location *loc_new, int t_loc) {
	int now = Time(this->tid_time);

	if (train_get_tspeed(train) + MS2TICK(3000) > now) return; // wait until the velocity settles

	train_simulate(train, now);

	fixed v = train_get_velocity(train);
	ASSERT(fixed_sgn(v) >= 0, "bad velocity %F", v);

	location loc;
	train_get_loc(train, &loc);

	fixed dt_lag = fixed_new(TICK2MS(now - t_loc));
	fixed dx_lag = fixed_mul(v, dt_lag);
	location_inc(loc_new, dx_lag);
	train_set_loc(train, loc_new);

	if (!location_isundef(&loc)) {
		fixed dist = location_dist_min(&loc, loc_new);
		ASSERT(fixed_sgn(dist) >= 0, "bad distance %F", dist);
		logdisplay_printf(this->log,
			"%-5s + %Fmm (%F)",
			loc_new->edge->src->name,
			loc_new->offset,
			dist
		);
		logdisplay_flushline(this->log);
	}
}

// @TODO: use actual history as opposed to guessing it with current loc/velocity
// @TODO: limiting the scope of this function makes it MUCH easier to code. right now the purpose is to only see where the train was at time t_past. this time is guaranteed to be near a sensor hit so there are no issues with track switch state.
void engineer_get_loc_hist(engineer *this, train_descriptor *train, int t_past, location *rv_loc) {
	train_get_loc(train, rv_loc); // estimate with current position (assume t_past is close to now)
	// int t = Time(this->tid_time);
	// ASSERT(t_past < t, "not in the past");
	// train_get_loc(train, rv_loc);
	// fixed v = train_get_velocity(train);
	// if (fixed_sgn(v) > 0) {
	// 	fixed dt = fixed_new(TICK2MS(t_past - t));
	// 	fixed dx = fixed_mul(v, dt);
	// 	location_inc(rv_loc, dx);
	// }
}

train_descriptor *engineer_attribute_loc(engineer *this, location *loc, int t_loc) {
	TRAIN_FOREACH(train_no) {
		train_descriptor *train = &this->train[train_no];
		if (fixed_sgn(train_get_velocity(train)) > 0) { // moving
			location loc_train;
			train_get_loc(train, &loc_train);
			if (!location_isundef(&loc_train)) { // known location
				location loc_past;
				engineer_get_loc_hist(this, train, t_loc, &loc_past);
				fixed dist = location_dist_min(&loc_past, loc);
				if (fixed_sgn(dist) >= 0 && fixed_cmp(dist, fixed_new(300)) <= 0) {
					return &this->train[train_no];
				}
			}
		}
	}
	train_descriptor *rv = NULL;
	TRAIN_FOREACH(train_no) {
		train_descriptor *train = &this->train[train_no];
		if (fixed_sgn(train_get_velocity(train)) > 0) { // moving
			location loc_train;
			train_get_loc(train, &loc_train);
			if (location_isundef(&loc_train)) { // unknown location
				if (rv) return NULL;
				rv = train;
			}
		}
	}
	return rv;
}

// @TODO: also use sensor OFF as it is just as accurate
void engineer_onsensor(engineer *this, char data[]) {
	engineer_ontick(this);
	msg_sensor *msg = (msg_sensor*) data;
	if (msg->state == OFF) return;
	track_node *sensor = engineer_get_tracknode(this, msg->module, msg->id);
	location loc_sensor = location_new(sensor->edge, fixed_new(0));
	train_descriptor *train = engineer_attribute_loc(this, &loc_sensor, msg->timestamp);
	if (train) {
		logdisplay_printf(this->log2, "attributing sensor %s to train %d", sensor->name, train->no);
		logdisplay_flushline(this->log2);
		engineer_train_on_loc(this, train, &loc_sensor, msg->timestamp);
	} else {
		logdisplay_printf(this->log2, "spurious sensor %s", sensor->name);
		logdisplay_flushline(this->log2);
	}
}

void engineer_ontick(engineer *this) {
	int t = Time(this->tid_time);
	TRAIN_FOREACH(train_no) {
		train_simulate(&this->train[train_no], t);
	}
}
