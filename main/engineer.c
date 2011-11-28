#include <engineer.h>
#include <syscall.h>
#include <server/traincmdbuffer.h>
#include <fixed.h>
#include <track_data.h>
#include <uconst.h>
#include <server/publisher.h>
#include <gps.h>

engineer *engineer_new(char track_name) {
	engineer *this = malloc(sizeof(engineer));

	// initialize helper tasks
	this->tid_traincmdbuf = traincmdbuffer_new();

	// start
	train_go(this->tid_traincmdbuf);

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

	gps *gps = gps_new(this->track_nodes_arr);
	// initialize train descriptors
	TRAIN_FOREACH(train_no) {
		train_state *train = engineer_get_train(this, train_no);
		train_init(train, train_no, gps);
	}

	this->con = console_new(COM2);
	this->log = logdisplay_new(this->con, 11, 56, 8, 100, ROUNDROBIN, "train location log");
	this->log2 = logdisplay_new(this->con, 11 + 9, 56, 8, 100, ROUNDROBIN, "location attribution log");
	this->triplog = logdisplay_new(this->con, 11 + 20, 56 + 15, 8 + 12, 100, ROUNDROBIN, "engineer triplog");
	this->tid_time = WhoIs(NAME_TIMESERVER);

	return this;
}

void engineer_destroy(engineer *this) {
	TRAIN_FOREACH(train_no) {
		train_state *train = &this->train[train_no];
		if (train_is_moving(train)) {
			engineer_set_speed(this, train_no, 0);
		}
	}
	train_stop(this->tid_traincmdbuf);
	Flush(WhoIs(NAME_IOSERVER_COM1));
}

train_state *engineer_get_train(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	return &this->train[train_no];
}

void engineer_on_set_speed(engineer *this, int train_no, int speed, int t) {
	train_state *train = engineer_get_train(this, train_no);
	if (!train->calibrated) return;
	train_set_speed(train, speed, t);
}

void engineer_set_speed(engineer *this, int train_no, int speed) {
	train_speed(train_no, speed, this->tid_traincmdbuf);
}

void engineer_on_reverse(engineer *this, int train_no, int t) {
	train_state *train = engineer_get_train(this, train_no);
	if (!train->calibrated) return;
	train_on_reverse(train, t);
}

// @TODO: this is a tricky thing because it's technically a small path involving 3 commands and 2 delays. this needs to be rethought in the context of a path finder.
void engineer_reverse(engineer *this, int train_no) {
	train_state *train = engineer_get_train(this, train_no);

	int speed = train_get_speed(train);
	// int dstop = train_get_stopdist(train);
	// fixed v = train_get_velocity(train);
	engineer_set_speed(this, train_no, 0);
	// if (fixed_sgn(v) > 0) {
	// 	fixed stoptime = fixed_div(fixed_mul(fixed_new(2), dstop), v);
	// 	stoptime = fixed_add(stoptime, fixed_new(MS2TICK(500)));
	// 	traincmdbuffer_put(this->tid_traincmdbuf, PAUSE, fixed_int(stoptime), NULL);
	// 	traincmdbuffer_put(this->tid_traincmdbuf, REVERSE, train_no, NULL);
	// 	traincmdbuffer_put(this->tid_traincmdbuf, PAUSE, TRAIN_PAUSE_AFTER_REVERSE, NULL);
	// } else {
	train_reverse(train_no, this->tid_traincmdbuf); // regular safe pause
	// }
	engineer_set_speed(this, train_no, speed);
}

void engineer_set_track(engineer *this, int s[], int ns, int c[], int nc) {
	while (ns--) {
		engineer_set_switch(this, *s++, 's');
	}
	while (nc--) {
		engineer_set_switch(this, *c++, 'c');
	}
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

track_node *engineer_get_track(engineer *this) {
	return this->track_nodes_arr;
}

void engineer_on_set_switch(engineer *this, int id, int pos, int t) {
	(void) t; // @TODO: perhaps save a timeline of switch states
	track_node *br = engineer_get_tracknode(this, "BR", id);
	int dir = POS2DIR(pos);
	br->switch_dir = dir;
}

void engineer_set_switch(engineer *this, int id, int pos) {
	// track_node *br = engineer_get_tracknode(this, "BR", id);
	// int dir = POS2DIR(pos);
	// if (br->switch_dir != dir) { // assume only engineer switches branches
		train_switch(id, pos, this->tid_traincmdbuf);
	// }
}

void engineer_train_on_loc(engineer *this, train_state *train, location *new_loc_pickup, int t_loc) {
	int now = Time(this->tid_time);

	train_update_simulation(train, now);

	location old_loc_pickup = train_get_pickuploc(train);

	fixed dx_lag = train_simulate_dx(train, t_loc, now);
	location_add(new_loc_pickup, dx_lag);
	train_set_pickuploc(train, new_loc_pickup);

	if (!train_is_lost(train)) {
		int dist = location_dist_min(&old_loc_pickup, new_loc_pickup);
		logdisplay_printf(this->log,
			"pred: %L, attrib: %L (%dmm)",
			&old_loc_pickup,
			&new_loc_pickup,
			dist
		);
		logdisplay_flushline(this->log);
	}
}

train_state *engineer_attribute_pickuploc(engineer *this, location *attr_loc_pickup, int t_loc) {
	int closest_dist = infinity;
	train_state *closest_train = NULL;

	TRAIN_FOREACH(train_no) {
		train_state *train = &this->train[train_no];
		if (!train->calibrated) continue;
		if (!train_is_moving(train)) continue;
		if (train_is_lost(train)) continue;

		location cur_loc_pickup = train_get_pickuploc(train);
		location old_loc_pickup = train_get_pickuploc_hist(train, t_loc);

		// @TODO: since we have a max range we should short circuit
		int dist = location_dist_min(&old_loc_pickup, attr_loc_pickup);

		if (dist >= 0) {
			if (dist < closest_dist) {
				closest_dist = dist;
				closest_train = train;
			}
			logdisplay_printf(this->log2,
				"testing %L vs train %d at %L (was at %L), distance: %dmm",
				&attr_loc_pickup,
				train_no,
				&cur_loc_pickup,
				&old_loc_pickup,
				dist
			);
		} else {
			logdisplay_printf(this->log2,
				"testing %L vs train %d at %L (was at %L), no path found",
				&attr_loc_pickup,
				train_no,
				&cur_loc_pickup,
				&old_loc_pickup
			);
		}
	}

	if (closest_train && closest_dist < 400) return closest_train; // @TODO: pull out magic constant

	train_state *rv = NULL;
	TRAIN_FOREACH(train_no) {
		train_state *train = &this->train[train_no];
		if (!train->calibrated) continue;
		if (!train_is_moving(train)) continue;
		if (!train_is_lost(train)) continue;

		if (rv) {
			logdisplay_printf(this->log2,
				"can't attribute %L, multiple moving lost trains",
				&attr_loc_pickup
			);
			logdisplay_flushline(this->log2);
			return NULL;
		}

		rv = train;
	}

	return rv;
}

void engineer_onloc(engineer *this, location *loc, int t_loc) {
	train_state *train = engineer_attribute_pickuploc(this, loc, t_loc);
	if (train) {
		logdisplay_printf(this->log2, "attributing %L to train %d", &loc, train->no);
		logdisplay_flushline(this->log2);
		engineer_train_on_loc(this, train, loc, t_loc);
	} else {
		logdisplay_printf(this->log2, "spurious location %L", &loc);
		logdisplay_flushline(this->log2);
	}
}

void engineer_onsensor(engineer *this, char data[]) {
	engineer_ontick(this);
	msg_sensor *msg = (msg_sensor*) data;
	track_node *sensor = engineer_get_tracknode(this, msg->module, msg->id);
	location loc_sensor = location_fromnode(sensor, DIR_AHEAD);
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
		train_state *train = &this->train[train_no];
		if (train->calibrated) {
			train_update_simulation(train, t);
			train_ontick(train, this->tid_traincmdbuf, this->track_nodes, this->triplog, t);
		}
	}
}
