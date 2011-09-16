#pragma once

enum switch_state { straight, curved };

void train_init();
void train_stop();
void train_go();
void train_setspeed(int train, int speed);
void train_reverse(int train);
void train_setswitch(int sw, enum switch_state state);



