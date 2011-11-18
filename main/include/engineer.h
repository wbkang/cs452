#pragma once

#include <track_node.h>
#include <train.h>
#include <lookup.h>
#include <fixed.h>
#include <location.h>
#include <console.h> // @TODO: replace for a ui task
#include <ui/logdisplay.h> // @TODO: replace for a ui task

typedef struct _tag_train_engineer {
	int tid_traincmdbuf;
	train_descriptor train[TRAIN_MAX_TRAIN_ADDR + 1];
	lookup *track_nodes;
	console *con; // @TODO: replace with a ui task tid
	logdisplay *log; // @TODO: replace with a ui task tid
	logdisplay *log2; // @TODO: replace with a ui task tid
	int tid_time;
} engineer;

engineer *engineer_new(char track_name);
void engineer_destroy(engineer *this);
train_direction engineer_train_get_dir(engineer *this, int train_no);
void engineer_train_set_dir(engineer *this, int train_no, train_direction dir);
void engineer_set_stopinfo(engineer *this, int train_no, fixed m, fixed b);
void engineer_get_stopinfo(engineer *this, int train_no, fixed *m, fixed *b);
fixed engineer_sim_stopdist(engineer *this, int train_no);
void engineer_set_speed(engineer *this, int train_no, int speed);
void engineer_on_set_speed(engineer *this, int train_no, int speed);
int engineer_get_speed(engineer *this, int train_no);
void engineer_set_velocity(engineer *this, int train_no, fixed v);
fixed engineer_get_velocity(engineer *this, int train_no);
void engineer_set_loc(engineer *this, int train_no, location *loc);
void engineer_get_loc(engineer *this, int train_no, location *loc);
void engineer_reverse(engineer *this, int train_no);
int engineer_get_speedidx(engineer *this, int train_no);
void engineer_train_pause(engineer *this, int train_no, int ticks);
void engineer_set_track(engineer *this, int s[], int ns, int c[], int nc);
track_node *engineer_get_tracknode(engineer *this, char *type, int id);
void engineer_set_switch(engineer *this, int id, int pos, int offsolenoid);
void engineer_train_onsensor(engineer *this, train_descriptor *train, track_node *sensor, int timestamp);
void engineer_onsensor(engineer *this, char data[]);
void engineer_ontick(engineer *this);
train_descriptor *engineer_attribute_sensor(engineer *this, track_node *sensor, int timestamp);
void engineer_train_move(engineer *this, train_descriptor *train, int t_i, int t_f);






