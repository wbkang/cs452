#pragma once

void train_speed(char train, char speed, int tid_com);
void train_reverse(char train, int tid_com, int tid_time);
void train_aux(char enable, char func, char train, int tid_com);
void train_switch(char switchaddr, char pos, int tid_com, int tid_time);
void train_switchall(char pos, int tid_com, int tid_time);
void train_solenoidoff(int tid_com);
void train_querysenmod(char module, int tid_com);
void train_querysenmods(char modules, int tid_com);
void train_go(int tid_com);
void train_stop(int tid_com);
