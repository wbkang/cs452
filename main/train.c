#include <train.h>
#include <syscall.h>

#define PAUSE_SOLENOID 15
#define PAUSE_REVERSE 300

void train_speed(char train, char speed, int tid_com) {
	ASSERT(train_goodtrain(train), "bad train: %d", train);
	Putc(speed, tid_com);
	Putc(train, tid_com);
}

void train_reverse(char train, int tid_com, int tid_time) {
	ASSERT(train_goodtrain(train), "bad train: %d", train);
	Delay(PAUSE_REVERSE, tid_time);
	Putc(0xF, tid_com);
	Putc(train, tid_com);
}

void train_switch(char switchaddr, char pos, int tid_com, int tid_time) {
	ASSERT(train_goodswitchpos(pos), "bad position: %d", pos);
	Putc(pos == 'S' ? 0x21 : 0x22, tid_com);
	Putc(switchaddr, tid_com);
	Delay(PAUSE_SOLENOID, tid_time);
}

void train_switchall(char pos, int tid_com, int tid_time) {
	for (int i = 1; i <= 18; i++) {
		train_switch(i, pos, tid_com, tid_time);
	}
	for (int i = 0x99; i <= 0x9C; i++) {
		train_switch(i, pos, tid_com, tid_time);
	}
	train_solenoidoff(tid_com);
}

void train_solenoidoff(int tid_com) {
	Putc(0x20, tid_com);
}

void train_querysenmod(char module, int tid_com) {
	ASSERT(train_goodmodule(module), "bad module: %d", module);
	Putc(0xC0 + module, tid_com);
}

void train_querysenmods(char modules, int tid_com) {
	ASSERT(train_goodmodule(modules), "bad module: %d", modules);
	Putc(0x80 + modules, tid_com);
}

void train_go(int tid_com) {
	Putc(0x60, tid_com);
}

void train_stop(int tid_com) {
	Putc(0x61, tid_com);
}
