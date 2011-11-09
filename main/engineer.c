#include <engineer.h>
#include <syscall.h>
#include <server/traincmdbuffer.h>
#include <server/traincmdrunner.h>
#include <fixed.h>
#include <track_data.h>
#include <stop_distance.h>
#include <betaimporter.h>

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
		train->stopm = 0;
		train->stopb = 0;
		train->speed = 0;
		train->last_speed = 0;
		train->direction = TRAIN_FORWARD;
		TRAIN_FOREACH_SPEEDIDX(speed) {
			train->tref[speed] = -1;
		}
	}

	// initialize track nodes
	track_node *tn = malloc(sizeof(track_node) * TRACK_MAX);
	switch (track_name) {
		case 'a':
			this->track_nodes = init_tracka(tn);
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

	return this;
}

void engineer_set_tref(engineer *this, int train_no, int speed_idx, int tref) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	ASSERT(0 <= speed_idx && speed_idx < TRAIN_NUM_SPEED_IDX, "bad speed_idx");
	this->train[train_no].tref[speed_idx] = tref;
	populate_stop_distance(&this->train[train_no], train_no);
}

int engineer_get_tref(engineer *this, int train_no, int speed_idx) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	ASSERT(0 <= speed_idx && speed_idx < TRAIN_NUM_SPEED_IDX, "bad speed_idx");
	return this->train[train_no].tref[speed_idx];
}

void engineer_set_stopinfo(engineer *this, int train_no, fixed m, fixed b) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	train->stopm = m;
	train->stopb = b;
}

fixed engineer_get_stopdist(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	fixed tref14 = train->tref[14];
	fixed trefnow = train->tref[train_speed2speed_idx(train)];

	if (trefnow != fixed_new(-1)) {
		return fixed_add(fixed_mul(train->stopm, fixed_div(fixed_mul(tref14, fixed_new(14)), trefnow)), train->stopb);
	} else {
		return fixed_add(fixed_mul(train->stopm, fixed_new(train->speed)), train->stopb);
	}
}

void engineer_get_stopinfo(engineer *this, int train_no, fixed *m, fixed *b) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	*m = train->stopm;
	*b = train->stopb;
}

void engineer_set_speed(engineer *this, int train_no, int speed) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_descriptor *train = &this->train[train_no];
	train->last_speed = train->speed;
	train->speed = speed;
	train_speed(train_no, speed, this->tid_traincmdbuf);
}

int engineer_get_speed(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	return this->train[train_no].speed;
}

void engineer_reverse(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	int speed = engineer_get_speed(this, train_no);
	engineer_set_speed(this, train_no, 0);
	train_reverse(train_no, this->tid_traincmdbuf);
	engineer_train_set_dir(this, train_no, opposite_direction(engineer_train_getdir(this, train_no)));
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
	traincmdbuffer_put(this->tid_traincmdbuf, PAUSE, ticks, NULL);
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
	return lookup_get(this->track_nodes, name);
}

void engineer_set_switch(engineer *this, int id, int pos, int offsolenoid) {
	track_node *br = engineer_get_tracknode(this, "BR", id);
	ASSERTNOTNULL(br);
	br->switch_dir = POS2DIR(pos);
	train_switch(id, pos, this->tid_traincmdbuf);
	if (offsolenoid) {
		train_solenoidoff(this->tid_traincmdbuf);
	}
}

void engineer_destroy(engineer *this) {
	TRAIN_FOREACH(train_no) {
		if (engineer_get_speed(this, train_no)) {
			engineer_set_speed(this, train_no, 0);
		}
	}
	train_stop(this->tid_traincmdbuf);
}

train_direction engineer_train_get_dir(engineer *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	return this->train[train_no].direction;
}

void engineer_train_set_dir(engineer *this, int train_no, train_direction dir) {
	this->train[train_no].direction = dir;
}
// @TODO: think this out. Would be nice to know current velocity
// void engineer_set_velocity(engineer *this, int train_no, fixed measured_v, fixed predicted_v) {
// 	if (measured_v == 0) return;
// 	if (abs(fixed_div(measure_v, predicted_v)) <= fixed_div(fixed_new(5), fixed_new(100)))
// 	if (fixed_div())
// 	this->valocity
// }
