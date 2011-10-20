#include <train.h>
#include <syscall.h>

#define PAUSE_SOLENOID 15
#define PAUSE_REVERSE 300

void train_speed(char train, char speed, int tid_com) {
	ASSERT(1 <= train && train <= 80, "bad train: %d", train);
	ASSERT(speed <= 14, "bad speed: %d", speed);
	Putc(speed, tid_com);
	Putc(train, tid_com);
}

void train_reverse(char train, int tid_com, int tid_time) {
	ASSERT(1 <= train && train <= 80, "bad train: %d", train);
	Delay(PAUSE_REVERSE, tid_time);
	Putc(0xF, tid_com);
	Putc(train, tid_com);
}

void train_aux(char enable, char func, char train, int tid_com) {
	ASSERT(enable <= 1, "bad enable: %d", enable);
	ASSERT(func <= 15, "bad func: %d", func);
	ASSERT(1 <= train && train <= 80, "bad train: %d", train);
	Putc((enable << 4) + func, tid_com);
	Putc(train, tid_com);
}

void train_switch(char switchaddr, char pos, int tid_com, int tid_time) {
	ASSERT(pos == 'S' || pos == 'C', "bad position: %d", pos);
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
	ASSERT(module > 0 && module < 32, "bad module: %d", module);
	Putc(0xC0 + module, tid_com);
}

void train_querysenmods(char modules, int tid_com) {
	ASSERT(modules > 0 && modules < 32, "bad module: %d", modules);
	Putc(0x80 + modules, tid_com);
}

void train_go(int tid_com) {
	Putc(0x60, tid_com);
}

void train_stop(int tid_com) {
	Putc(0x61, tid_com);
}
