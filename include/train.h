#pragma once

#define NUM_SWITCHES 0x9e // 0 index

enum switch_state { straight, curved };

void train_init();
void train_stop();
void train_go();
void train_setspeed(int train, int speed);
void train_reverse(int train);
void train_setswitch(int sw, enum switch_state state);

enum switch_state train_getswitch(int sw);

void train_batch_sensor_req(int n);
