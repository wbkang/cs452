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
	dumbbus *time_bus;
	// train data
	engineer *eng;
	// these are not scalable to multiple trains
	int cur_train;
	uint cur_tick;
	track_node *cur_node;
	uint last_tick;
	track_node *last_node;
	uint trial;
} a0state;

void a0();

void ui_set_track(a0state *state, int s[], int ns, int c[], int nc);
