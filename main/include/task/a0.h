#pragma once

#include <console.h>
#include <ui/logdisplay.h>
#include <ui/logstrip.h>
#include <ui/cmdline.h>
#include <dumbbus.h>
#include <ui/timedisplay.h>
#include <ui/a0_track_template.h>
#include <ui/a0ui.h>
#include <engineer.h>
#include <track_node.h>

typedef struct a0state a0state;
struct a0state {
	// server ids
	int tid_time;
	ui_id id_ui;
	// ui
	a0ui *a0ui;
	dumbbus *sensor_bus;
	dumbbus *bus10hz; // a0ui, time in ticks
	dumbbus *simbus; // engineer, a0state
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
