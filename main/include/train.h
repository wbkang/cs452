#pragma once

#include <fixed.h>
#include <syscall.h>
#include <hardware.h>
#include <uconst.h>
#include <track_node.h>
#include <location.h>
#include <gps.h>
#include <track_reservation.h>
#include <lookup.h>
#include <server/traincmdbuffer.h>
#include <ui/logdisplay.h>

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
#define TRAIN_GOOD_SPEED(x) (TRAIN_MIN_SPEED <= (x) && (x) <= TRAIN_MAX_SPEED)
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
#define TRAIN_MAX_VCMD 40

typedef enum {
	TRAIN_UNKNOWN, TRAIN_FORWARD, TRAIN_BACKWARD
} train_direction;

typedef struct train_descriptor train_descriptor;
struct train_descriptor {
	// static data
	int no;
	fixed stopm; // 0 if unknown
	fixed stopb; // 0 if unknown
	fixed len_pickup;
	fixed dist2nose;
	fixed dist2tail;
	fixed v_avg[TRAIN_NUM_SPEED_IDX];
	fixed ai_avg10000;
	fixed ad_avg10000;
	int calibrated;

	fixed a_m10000;
	fixed a_b10000;
	// dynamic data
	train_direction dir;
	fixed v10000;
	fixed v_i10000;
	fixed v_f10000;
	int speed;
	int t_speed;
	int last_speed;
	location loc;
	int t_sim;
	// vcmd stuff
	struct gps *gps;
	int vcmdidx;
	trainvcmd *vcmds;
	trainvcmd *last_run_vcmd;
	int vcmdslen;
	int vcmdwait;
	location destination;
	struct path *path;
};

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

static inline int train_speed2speedidx(int lastspeed, int curspeed) {
	if (curspeed <= lastspeed || curspeed == TRAIN_MAX_SPEED) {
		return curspeed;
	} else {
		return curspeed + TRAIN_MAX_SPEED;
	}
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
	traincmdbuffer_put(tid, REVERSE_UI, train, NULL);
}

static inline void train_switch(char no, char pos, int tid) {
	ASSERT(track_switchpos_isgood(pos), "bad position: %d", pos);
	traincmdbuffer_put(tid, SWITCH, no, pos);
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

void train_init_static(train_descriptor *this);
void train_init(train_descriptor *this, int no, struct gps *gps);
fixed train_get_velocity(train_descriptor *this);
fixed train_get_stopdist4speedidx(train_descriptor *this, int speed_idx);
fixed train_get_stopdist(train_descriptor *this);
int train_get_speed(train_descriptor *this);
int train_get_speedidx(train_descriptor *this);
void train_set_speed(train_descriptor *this, int speed, int t);
void train_get_loc(train_descriptor *this, location *loc);
void train_set_loc(train_descriptor *this, location *loc);
train_direction train_get_dir(train_descriptor *this);
void train_set_dir(train_descriptor *this, train_direction dir);
void train_reverse_dir(train_descriptor *this);
void train_on_reverse(train_descriptor *this, int t);
int train_get_tspeed(train_descriptor *this);
void train_set_tspeed(train_descriptor *this, int t_speed);
int train_get_tsim(train_descriptor *this);
void train_set_tsim(train_descriptor *this, int t_sim);
int train_get_length(train_descriptor *this);
fixed train_simulate_dx(train_descriptor *this, int t_i, int t_f);
void train_get_loc_hist(train_descriptor *this, int t_i, location *rv_loc);
void train_update_simulation(train_descriptor *this, int t_f);
void train_set_dest(train_descriptor *this, location *dest);
void train_run_vcmd(train_descriptor *this, int tid_traincmdbuf, lookup *nodemap, logdisplay *log, int tick);
int train_get_reverse_cost(train_descriptor *this, int dist);
int train_get_train_length(train_descriptor *this);
