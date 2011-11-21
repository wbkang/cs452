#include <engineer.h>
#include <syscall.h>
#include <server/traincmdbuffer.h>
#include <fixed.h>
#include <track_data.h>
#include <uconst.h>
#include <server/publisher.h>

typedef struct gps gps;
void gps_test(gps *this);


engineer *engineer_new(char track_name) {
	engineer *this = malloc(sizeof(engineer));

	// initialize helper tasks
	this->tid_traincmdbuf = traincmdbuffer_new();

	// start
	train_go(this->tid_traincmdbuf);

	// initialize train descriptors
	TRAIN_FOREACH(train_no) {
		train_init(&this->train[train_no], train_no);
	}

	// initialize track object
	this->track_nodes_arr = malloc(sizeof(track_node) * TRACK_MAX);
	switch (track_name) {
		case 'a':
			this->track_nodes = init_tracka(this->track_nodes_arr);
			break;
		case 'b':
			this->track_nodes = init_trackb(this->track_nodes_arr);
			break;
		default:
			ASSERT(0, "bad track name");
			break;
	}

	this->con = console_new(COM2);
	this->log = logdisplay_new(this->con, 11, 56, 8, 80, ROUNDROBIN);
	this->log2 = logdisplay_new(this->con, 11 + 9, 56, 8, 80, ROUNDROBIN);
	this->tid_time = WhoIs(NAME_TIMESERVER);
	this->reservation = track_reservation_new();

	gps *g = gps_new(this->track_nodes_arr);
	gps_test(g);
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

void engineer_on_set_speed(engineer *this, int train_no, int speed, int t) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_set_speed(&this->train[train_no], speed, t);
}

void engineer_set_speed(engineer *this, int train_no, int speed) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_speed(train_no, speed, this->tid_traincmdbuf);
}

void engineer_get_loc(engineer *this, int train_no, location *loc) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_get_loc(&this->train[train_no], loc);
}

// @TODO: this is a tricky thing because it's technically a small path involving 3 commands and 2 delays. this needs to be rethought in the context of a path finder.
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

track_node *engineer_get_tracknodearr(engineer *this) {
	return this->track_nodes_arr;
}

void engineer_on_set_switch(engineer *this, int id, int pos) {
	track_node *br = engineer_get_tracknode(this, "BR", id);
	int dir = POS2DIR(pos);
	br->switch_dir = dir;
}

void engineer_set_switch(engineer *this, int id, int pos, int offsolenoid) {
	track_node *br = engineer_get_tracknode(this, "BR", id);
	int dir = POS2DIR(pos);
	if (br->switch_dir != dir) { // assume only engineer switches branches
		train_switch(id, pos, this->tid_traincmdbuf);
	}
	if (offsolenoid) {
		train_solenoidoff(this->tid_traincmdbuf);
	}
}

train_direction engineer_train_get_dir(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	return train_get_dir(&this->train[train_no]);
}

void engineer_train_set_dir(engineer *this, int train_no, train_direction dir) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_set_dir(&this->train[train_no], dir);
}

void engineer_train_on_loc(engineer *this, train_descriptor *train, location *loc_new, int t_loc) {
	int now = Time(this->tid_time);

	if (train_get_tspeed(train) + MS2TICK(5000) > now) return; // wait until the velocity settles

	train_update_simulation(train, now);

	location loc;
	train_get_loc(train, &loc);

	fixed dx_lag = train_simulate_dx(train, t_loc, now);
	location_add(loc_new, dx_lag);
	train_set_loc(train, loc_new);

	if (!location_isundef(&loc)) {
		fixed dist = location_dist_min(&loc, loc_new);
		ASSERT(fixed_sgn(dist) >= 0, "bad distance %F", dist);
		logdisplay_printf(this->log,
			"pred: %-5s + %Fmm, attrib: %-5s + %Fmm (%F)",
			loc.edge->src->name,
			loc.offset,
			loc_new->edge->src->name,
			loc_new->offset,
			dist
		);
		logdisplay_flushline(this->log);
	}
}

train_descriptor *engineer_attribute_loc(engineer *this, location *loc, int t_loc) {
	TRAIN_FOREACH(train_no) {
		train_descriptor *train = &this->train[train_no];
		if (fixed_sgn(train_get_velocity(train)) > 0) { // moving
			location loc_train;
			train_get_loc(train, &loc_train);
			if (!location_isundef(&loc_train)) { // known location
				location loc_past;
				train_get_loc_hist(train, t_loc, &loc_past);
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

void engineer_onloc(engineer *this, location *loc, int t_loc) {
	train_descriptor *train = engineer_attribute_loc(this, loc, t_loc);
	if (train) {
		logdisplay_printf(this->log2,
			"attrib. loc. %s + %Fmm to train %d",
			loc->edge->src->name,
			loc->offset,
			train->no
		);
		logdisplay_flushline(this->log2);
		engineer_train_on_loc(this, train, loc, t_loc);
	} else {
		logdisplay_printf(this->log2, "spurious sensor %s", loc->edge->src->name);
		logdisplay_flushline(this->log2);
	}
}

void engineer_onsensor(engineer *this, char data[]) {
	engineer_ontick(this);
	msg_sensor *msg = (msg_sensor*) data;
	track_node *sensor = engineer_get_tracknode(this, msg->module, msg->id);
	location loc_sensor = location_new(sensor->edge);
	if (msg->state == OFF) {
		return; // @TODO: removing this reduces simulation accuracy, why?
		fixed len_pickup = fixed_new(50); // @TODO: don't hardcode, even though so easy
		location_add(&loc_sensor, len_pickup);
	}
	engineer_onloc(this, &loc_sensor, msg->timestamp);
}

void engineer_ontick(engineer *this) {
	int t = Time(this->tid_time);
	TRAIN_FOREACH(train_no) {
		train_update_simulation(&this->train[train_no], t);
	}
}
