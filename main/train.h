#pragma once

static inline int train_goodtrain(int train) {
	return (1 <= train && train <= 80);
}

static inline int train_goodspeed(int speed) {
	return speed >= 0;
}

static inline int train_goodswitch(int switchno) {
	return 1; // switchno > NUM_SWITCHES || switches[switchno] == 0;
}

static inline int train_goodswitchpos(int pos) {
	return (pos == 'S' || pos == 'C');
}

static inline int train_goodmodule(int module) {
	return (module > 0 && module < 32);
}

void train_speed(char train, char speed, int tid);
void train_reverse(char train, int tid);
void train_switch(char switchaddr, char pos, int tid);
void train_switchall(char pos, int tid);
void train_solenoidoff(int tid);
void train_querysenmod(char module, int tid);
void train_querysenmods(char modules, int tid);
void train_go(int tid);
void train_stop(int tid);
