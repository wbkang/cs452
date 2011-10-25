#pragma once

#include <traincmdbuffer.h>
#include <syscall.h>
#include <hardware.h>

#define TRAIN_MIN_TRAIN_ADDR 1
#define TRAIN_MAX_TRAIN_ADDR 80

#define TRAIN_MIN_SWITCHADDR 0
#define TRAIN_MAX_SWITCHADDR 255

#define TRAIN_PAUSE_SOLENOID 15
#define TRAIN_PAUSE_REVERSE 400

#define TRAIN_NUM_SWITCHADDR 22

static const char train_switches[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	14, 15, 16, 17, 18, 0x99, 0x9A, 0x9B, 0x9C
};

static inline int train_goodtrain(int train) {
	return TRAIN_MIN_TRAIN_ADDR <= train && train <= TRAIN_MAX_TRAIN_ADDR;
}

static inline int train_goodspeed(int speed) {
	return speed >= 0;
}

static inline int train_goodswitch(int switchno) {
	return 1; // TRAIN_MIN_SWITCHADDR <= switchno && switchno <= TRAIN_MAX_SWITCHADDR;
}

static inline int train_goodswitchpos(int pos) {
	return pos == 's' || pos == 'S' || pos == 'c' || pos == 'C';
}

static inline int train_goodmodule(int module) {
	return module > 0 && module < 32;
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
	if (pos == 'c') {
		pos = 'C';
	} else if (pos == 's') {
		pos = 'S';
	}
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
	for (int i = 1; i < TRAIN_NUM_SWITCHADDR; i++) {
		train_switch(train_switches[i], pos, tid);
	}
	train_solenoidoff(tid);
}
