#pragma once

#include <track_node.h>
#include <util.h>
#include <console.h>
#include <dumbbus.h>
#include <lookup.h>
#include <ui/logstrip.h>
#include <ui/cmdline.h>
#include <ui/logdisplay.h>
#include <engineer.h>

typedef struct {
	// server ids
	int tid_time;
	int tid_com2;
	console *con;
	// ui
	logstrip *cmdlog;
	cmdline *cmdline;
	logstrip *sensorlog;
	logdisplay *log;
	logdisplay *expected_time_display;
	logstrip *landmark_display;
	logstrip *stopinfo;
	dumbbus *sensor_bus;
	dumbbus *bus10hz;
	dumbbus *simbus;
	int timestamp;
	// train data
	engineer *eng;
	// these are not scalable to multiple trains
	int cur_train;
	uint timestamp_cur_sensor;
	track_node *cur_sensor;
	uint timestamp_last_sensor;
	track_node *last_sensor;
	uint trial;
	// timers
	int tid_refresh;
	int tid_simstep;
} a0state;

void a0();

void ui_set_track(a0state *state, int s[], int ns, int c[], int nc);
