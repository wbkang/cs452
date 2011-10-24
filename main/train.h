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

void train_speed(char train, char speed, int tid_com);
void train_reverse(char train, int tid_com, int tid_time);
void train_switch(char switchaddr, char pos, int tid_com, int tid_time);
void train_switchall(char pos, int tid_com, int tid_time);
void train_solenoidoff(int tid_com);
void train_querysenmod(char module, int tid_com);
void train_querysenmods(char modules, int tid_com);
void train_go(int tid_com);
void train_stop(int tid_com);
