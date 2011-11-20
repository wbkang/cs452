#pragma once

#include <fixed.h>
#include <syscall.h>
#include <hardware.h>
#include <uconst.h>
#include <track_node.h>
#include <location.h>
#include <server/traincmdbuffer.h>

// train controller commands

#define TRAIN_REVERSE 0xF
#define TRAIN_SWITCH_STRAIGHT 0x21
#define TRAIN_SWITCH_CURVED 0x22
#define TRAIN_SOLENOID_OFF 0x20
#define TRAIN_QUERYMOD 0xC0
#define TRAIN_QUERYMODS 0x80
#define TRAIN_GO 0x60
#define TRAIN_STOP 0x61

// train track data

#define TRAIN_MIN_SPEED 0
#define TRAIN_MAX_SPEED 14
#define TRAIN_NUM_SPEED_IDX (TRAIN_MAX_SPEED * 2)
#define TRAIN_FOREACH_SPEED(x) for (int (x) = TRAIN_MIN_SPEED; (x) <= TRAIN_MAX_SPEED; (x)++)
#define TRAIN_FOREACH_SPEEDIDX(x) for (int (x) = TRAIN_MIN_SPEED; (x) < TRAIN_NUM_SPEED_IDX; (x)++)
#define TRAIN_MIN_TRAIN_ADDR 1
#define TRAIN_MAX_TRAIN_ADDR 80
#define TRAIN_GOODNO(x) (TRAIN_MIN_TRAIN_ADDR <= (x)) && ((x) <= TRAIN_MAX_TRAIN_ADDR)
#define TRAIN_FOREACH(x) for (int (x) = TRAIN_MIN_TRAIN_ADDR; (x) <= TRAIN_MAX_TRAIN_ADDR; (x)++)
#define TRAIN_MIN_SWITCHADDR 0
#define TRAIN_MAX_SWITCHADDR 255
#define TRAIN_PAUSE_SOLENOID MS2TICK(150)
#define TRAIN_PAUSE_REVERSE MS2TICK(4000)
#define TRAIN_PAUSE_AFTER_REVERSE MS2TICK(100)
#define TRAIN_NUM_MODULES 5
#define TRAIN_NUM_SENSORS 16
#define TRAIN_NUM_SWITCHADDR 22

typedef enum {
	TRAIN_UNKNOWN, TRAIN_FORWARD, TRAIN_BACKWARD
} train_direction;

typedef struct {
	int no;
	fixed stopm; // 0 if unknown
	fixed stopb; // 0 if unknown
	int speed;
	int t_speed;
	int last_speed;
	train_direction dir;
	fixed len_pickup;
	fixed dist2nose;
	fixed dist2tail;
	location loc;
	uint v_d[TRAIN_NUM_SPEED_IDX];
	uint v_t[TRAIN_NUM_SPEED_IDX];
	int t_sim;
} train_descriptor;

static inline int train_speed2speed_idx(train_descriptor *td) {
	int lastspeed = td->last_speed;
	int curspeed = td->speed;
	int rv;

	if (curspeed <= lastspeed || curspeed == TRAIN_MAX_SPEED) {
		// descending or equal case
		rv = curspeed;
	} else {
		// ascending case
		rv = curspeed + TRAIN_MAX_SPEED;
	}
	ASSERT(rv < TRAIN_NUM_SPEED_IDX, "rv = %d, last: %d, cur: %d", rv, td->last_speed, td->speed);
	return rv;
}

static inline int train_speed_idx2speed(int speed_idx) {
	if (speed_idx <= TRAIN_MAX_SPEED) {
		return speed_idx;
	} else {
		return speed_idx - TRAIN_MAX_SPEED;
	}
}

static inline int train_switchi2no(int i) {
	ASSERT(0 <= i && i < TRAIN_NUM_SWITCHADDR, "bad i");
	if (i < 18) return i + 1;
	return i + (0x99 - 18);
}

static inline int train_switchno2i(int n) {
	ASSERT((1 <= n && n <= 18) || (0x99 <= n && n <= 0x9C), "bad switchno");
	if (n <= 18) return n - 1;
	return n - (0x99 - 18);
}

static inline int train_goodtrain(int train) {
	return TRAIN_MIN_TRAIN_ADDR <= train && train <= TRAIN_MAX_TRAIN_ADDR;
}

static inline int train_goodspeed(int speed) {
	return speed >= 0 && speed <= TRAIN_MAX_SPEED;
}

static inline int train_goodswitch(int switchno) {
	// TRAIN_MIN_SWITCHADDR <= switchno && switchno <= TRAIN_MAX_SWITCHADDR;
	return 1;
}

static inline int train_goodmodule(int module) {
	return 0 < module && module < 32;
}

static inline void train_speed(char train, char speed, int tid) {
	ASSERT(train_goodtrain(train), "bad train: %d", train);
	traincmdbuffer_put(tid, SPEED, train, speed);
}

static inline void train_reverse(char train, int tid) {
	ASSERT(train_goodtrain(train), "bad train: %d", train);
	traincmdbuffer_put(tid, PAUSE, TRAIN_PAUSE_REVERSE, NULL);
	traincmdbuffer_put(tid, REVERSE, train, NULL);
	traincmdbuffer_put(tid, PAUSE, TRAIN_PAUSE_AFTER_REVERSE, NULL); // @TODO: why?
}

static inline void train_switch(char no, char pos, int tid) {
	ASSERT(train_goodswitchpos(pos), "bad position: %d", pos);
	traincmdbuffer_put(tid, SWITCH, no, pos);
	traincmdbuffer_put(tid, PAUSE, TRAIN_PAUSE_SOLENOID, NULL);
}

static inline void train_solenoidoff(int tid) {
	traincmdbuffer_put(tid, SOLENOID, NULL, NULL);
}

static inline void train_querysenmod(char module, int tid) {
	ASSERT(train_goodmodule(module), "bad module: %d", module);
	traincmdbuffer_put(tid, QUERY1, module, NULL);
}

static inline void train_querysenmods(char modules, int tid) {
	ASSERT(train_goodmodule(modules), "bad module: %d", modules);
	traincmdbuffer_put(tid, QUERY, modules, NULL);
}

static inline void train_go(int tid) {
	traincmdbuffer_put(tid, GO, NULL, NULL);
}

static inline void train_stop(int tid) {
	traincmdbuffer_put(tid, STOP, NULL, NULL);
}

/*
 * Train object
 */

void train_data_populate(train_descriptor *train);
void train_init(train_descriptor *this, int no);
fixed train_get_velocity(train_descriptor *this);
fixed train_get_stopdist(train_descriptor *this);
int train_get_speed(train_descriptor *this);
void train_on_set_speed(train_descriptor *this, int speed, int t);
void train_get_loc(train_descriptor *this, location *loc);
void train_set_loc(train_descriptor *this, location *loc);
train_direction train_get_dir(train_descriptor *this);
void train_set_dir(train_descriptor *this, train_direction dir);
void train_reverse_dir(train_descriptor *this);
int train_get_tspeed(train_descriptor *this);
void train_set_tspeed(train_descriptor *this, int t_speed);
int train_get_tsim(train_descriptor *this);
void train_set_tsim(train_descriptor *this, int t_sim);
void train_simulate(train_descriptor *this, int t_f);
