#pragma once

#include <fixed.h>
#include <syscall.h>
#include <hardware.h>
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
#define TRAIN_MAX_SPEED 30
#define TRAIN_FOREACH_SPEED(x) for (int (x) = TRAIN_MIN_SPEED; (x) <= TRAIN_MAX_SPEED; (x)++)
#define TRAIN_MIN_TRAIN_ADDR 1
#define TRAIN_MAX_TRAIN_ADDR 80
#define TRAIN_FOREACH(x) for (int (x) = TRAIN_MIN_TRAIN_ADDR; (x) <= TRAIN_MAX_TRAIN_ADDR; (x)++)
#define TRAIN_MIN_SWITCHADDR 0
#define TRAIN_MAX_SWITCHADDR 255
#define TRAIN_PAUSE_SOLENOID 15
#define TRAIN_PAUSE_REVERSE 400
#define TRAIN_NUM_MODULES 5
#define TRAIN_NUM_SENSORS 16
#define TRAIN_NUM_SWITCHADDR 22

typedef struct {
	fixed tref[TRAIN_MAX_SPEED]; // -1 if unknown
} train_descriptor;

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

static inline int train_switchpos_straight(int pos) {
	return pos == 's' || pos == 'S';
}

static inline int train_switchpos_curved(int pos) {
	return pos == 'c' || pos == 'C';
}

static inline int train_goodswitchpos(int pos) {
	return train_switchpos_straight(pos) || train_switchpos_curved(pos);
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
}

static inline void train_switch(char switchaddr, char pos, int tid) {
	ASSERT(train_goodswitchpos(pos), "bad position: %d", pos);
	traincmdbuffer_put(tid, SWITCH, switchaddr, pos);
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

// extra

static inline void train_switchall(char pos, int tid) {
	for (int i = 0; i < TRAIN_NUM_SWITCHADDR; i++) {
		train_switch(train_switchi2no(i), pos, tid);
	}
}
