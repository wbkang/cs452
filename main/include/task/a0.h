#pragma once

#include <track_node.h>
#include <util.h>
#include <console.h>
#include <dumbbus.h>
#include <lookup.h>
#include <ui/logstrip.h>
#include <ui/cmdline.h>
#include <ui/logdisplay.h>
#include <ui/timedisplay.h>
#include <ui/a0_track_template.h>
#include <engineer.h>

typedef struct {
	// server ids
	int tid_time;
	console *con;
	// ui
	logdisplay *log;
	logstrip *cmdlog;
	logstrip *sensorlog;
	logstrip *trainloc1;
	logstrip *trainloc1r;
	logstrip *trainloc2;
	logstrip *trainloc2r;
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
	int cur_train;
	track_node *cur_sensor;
	track_node *last_sensor;
	// timers
	int tid_refresh;
	int tid_simstep;
	int tid_printloc;
} a0state;

void a0();

void ui_set_track(a0state *state, int s[], int ns, int c[], int nc);
