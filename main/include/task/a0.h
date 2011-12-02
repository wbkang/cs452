#pragma once

#include <console.h>
#include <ui/logdisplay.h>
#include <ui/logstrip.h>
#include <ui/cmdline.h>
#include <dumbbus.h>
#include <ui/timedisplay.h>
#include <ui/a0_track_template.h>
#include <engineer.h>
#include <track_node.h>

typedef struct a0state a0state;
struct a0state {
	// server ids
	int tid_time;
	console *con;
	// ui
	logdisplay *log;
	logstrip *cmdlog;
	logstrip *sensorlog;
	logstrip *train1info[3];
	logstrip *train2info[3];
	cmdline *cmdline;
	dumbbus *sensor_bus;
	dumbbus *bus10hz;
	dumbbus *simbus;
	dumbbus *printlocbus;
	timedisplay *timedisplay;
	track_template *template;
	int timestamp;
	// train data
	engineer *eng;
	// these are not scalable to multiple trains
	track_node *cur_sensor;
	track_node *last_sensor;
	// timers
	int tid_refresh;
	int tid_simstep;
	int tid_printloc;
};

a0state* get_state();

void a0();
void a0_destroy();
