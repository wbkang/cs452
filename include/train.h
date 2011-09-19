#pragma once

#define NUM_SWITCHES 0xff // 0 index

enum switch_state { straight, curved };

void train_init();
void train_stop();
void train_go();
void train_setspeed(int train, int speed);
void train_reverse(int train);
void train_setswitch(int sw, enum switch_state state);

enum switch_state train_getswitch(int sw);

// do not use
void train_batch_sensor_req(int n);
void train_sensor_req(int n); 
