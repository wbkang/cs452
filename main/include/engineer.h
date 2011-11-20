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
fixed engineer_sim_stopdist(engineer *this, int train_no);
void engineer_on_set_speed(engineer *this, int train_no, int speed, int t);
void engineer_set_speed(engineer *this, int train_no, int speed);
void engineer_get_loc(engineer *this, int train_no, location *loc);
void engineer_reverse(engineer *this, int train_no);
void engineer_train_pause(engineer *this, int train_no, int ticks);
void engineer_set_track(engineer *this, int s[], int ns, int c[], int nc);
track_node *engineer_get_tracknode(engineer *this, char *type, int id);
void engineer_set_switch(engineer *this, int id, int pos, int offsolenoid);
train_direction engineer_train_get_dir(engineer *this, int train_no);
void engineer_train_set_dir(engineer *this, int train_no, train_direction dir);
void engineer_train_on_loc(engineer *this, train_descriptor *train, location *loc_new, int t_loc);
void engineer_get_loc_hist(engineer *this, train_descriptor *train, int t_past, location *rv_loc);
train_descriptor *engineer_attribute_loc(engineer *this, location *loc, int t_loc);
void engineer_onloc(engineer *this, location *loc, int t);
void engineer_onsensor(engineer *this, char data[]);
void engineer_ontick(engineer *this);
