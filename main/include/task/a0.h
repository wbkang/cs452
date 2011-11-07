#pragma once

#include <track_node.h>
#include <util.h>
#include <train.h>
#include <console.h>
#include <dumbbus.h>
#include <lookup.h>
#include <ui/logstrip.h>
#include <ui/cmdline.h>


typedef struct {
	// server ids
	int tid_time;
	int tid_com1;
	int tid_com2;
	console *con;
	int tid_traincmdbuf;
	// ui
	int console_dump_line;
	logstrip logstrip;
	cmdline *cmdline;
	dumbbus *sensor_listeners;
	// train data
	char train_speed[TRAIN_MAX_TRAIN_ADDR + 1];
	train_descriptor train_desc[TRAIN_MAX_TRAIN_ADDR + 1];
	// track data
	lookup *nodemap;
	// these are not scalable to multiple trains
	uint cur_tick;
	track_node *cur_node;
	uint last_tick;
	track_node *last_node;
	uint trial;
	// sensor expectation data
	int linecnt;
} a0state;

void a0();
