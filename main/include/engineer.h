#pragma once

#include <track_node.h>
#include <train.h>
#include <lookup.h>
#include <fixed.h>

typedef struct _tag_train_engineer {
	int tid_traincmdbuf;
	train_descriptor train[TRAIN_MAX_TRAIN_ADDR + 1];
	lookup *track_nodes;
} engineer;


engineer *engineer_new(char track_name);
train_direction engineer_train_get_dir(engineer *this, int train_no);
void engineer_train_set_dir(engineer *this, int train_no, train_direction dir);
void engineer_set_tref(engineer *this, int train_no, int speed_idx, int tref);
int engineer_get_tref(engineer *this, int train_no, int speed_idx);
void engineer_set_stopinfo(engineer *this, int train_no, fixed m, fixed b);
void engineer_get_stopinfo(engineer *this, int train_no, fixed *m, fixed *b);
fixed engineer_get_stopdist(engineer *this, int train_no);
int engineer_get_dref(engineer *this, int train_no);
void engineer_set_dref(engineer *this, int train_no, int dref);
void engineer_set_speed(engineer *this, int train_no, int speed);
int engineer_get_speed(engineer *this, int train_no);
void engineer_reverse(engineer *this, int train_no);
int engineer_get_speedidx(engineer *this, int train_no);
void engineer_pause_train(engineer *this, int train_no, int ticks);
void engineer_set_track(engineer *this, int s[], int ns, int c[], int nc);
track_node *engineer_get_tracknode(engineer *this, char *type, int id);
void engineer_set_switch(engineer *this, int id, int pos, int offsolenoid);
void engineer_destroy(engineer *this);
