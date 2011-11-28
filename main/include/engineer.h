#pragma once

#include <track_node.h>
#include <train.h>
#include <lookup.h>
#include <fixed.h>
#include <location.h>
#include <console.h> // @TODO: replace for a ui task
#include <ui/logdisplay.h> // @TODO: replace for a ui task
#include <gps.h>

typedef struct _tag_train_engineer {
	int tid_traincmdbuf;
	train train[TRAIN_MAX_TRAIN_ADDR + 1];
	lookup *track_nodes;
	track_node *track_nodes_arr;
	console *con; // @TODO: replace with a ui task tid
	logdisplay *log; // @TODO: replace with a ui task tid
	logdisplay *log2; // @TODO: replace with a ui task tid
	logdisplay *triplog; // @TODO: replace with a ui task tid
	int tid_time;
} engineer;

engineer *engineer_new(char track_name);
void engineer_destroy(engineer *this);
train *engineer_get_train(engineer *this, int train_no);
void engineer_on_set_speed(engineer *this, int train_no, int speed, int t);
void engineer_set_speed(engineer *this, int train_no, int speed);
void engineer_on_reverse(engineer *this, int train_no, int t);
void engineer_reverse(engineer *this, int train_no);
void engineer_set_track(engineer *this, int s[], int ns, int c[], int nc);
track_node *engineer_get_tracknode(engineer *this, char *type, int id);
track_node *engineer_get_track(engineer *this);
void engineer_on_set_switch(engineer *this, int id, int pos, int t);
void engineer_set_switch(engineer *this, int id, int pos);
void engineer_train_on_loc(engineer *this, train *train, location *loc_new, int t_loc);
train *engineer_attribute_pickuploc(engineer *this, location *loc, int t_loc);
void engineer_onloc(engineer *this, location *loc, int t_loc);
void engineer_onsensor(engineer *this, char data[]);
void engineer_ontick(engineer *this);
