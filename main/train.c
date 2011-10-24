#include <train.h>
#include <traincmdbuffer.h>
#include <syscall.h>
#include <hardware.h>

#define PAUSE_SOLENOID 15
#define PAUSE_REVERSE 400

void train_speed(char train, char speed, int tid) {
	ASSERT(train_goodtrain(train), "bad train: %d", train);
	traincmdbuffer_put(tid, SPEED, train, speed);
}

void train_reverse(char train, int tid) {
	ASSERT(train_goodtrain(train), "bad train: %d", train);
	traincmdbuffer_put(tid, PAUSE, PAUSE_REVERSE, NULL);
	traincmdbuffer_put(tid, REVERSE, train, NULL);
}

void train_switch(char switchaddr, char pos, int tid) {
	ASSERT(train_goodswitchpos(pos), "bad position: %d", pos);
	traincmdbuffer_put(tid, SWITCH, switchaddr, pos);
	traincmdbuffer_put(tid, PAUSE, PAUSE_SOLENOID, NULL);
}

void train_switchall(char pos, int tid) {
	for (int i = 1; i <= 18; i++) {
		train_switch(i, pos, tid);
	}
	for (int i = 0x99; i <= 0x9C; i++) {
		train_switch(i, pos, tid);
	}
	train_solenoidoff(tid);
}

void train_solenoidoff(int tid) {
	traincmdbuffer_put(tid, SOLENOID, NULL, NULL);
}

void train_querysenmod(char module, int tid) {
	ASSERT(train_goodmodule(module), "bad module: %d", module);
	traincmdbuffer_put(tid, QUERY1, module, NULL);
}

void train_querysenmods(char modules, int tid) {
	ASSERT(train_goodmodule(modules), "bad module: %d", modules);
	traincmdbuffer_put(tid, QUERY, modules, NULL);
}

void train_go(int tid) {
	traincmdbuffer_put(tid, GO, NULL, NULL);
}

void train_stop(int tid) {
	traincmdbuffer_put(tid, STOP, NULL, NULL);
}
