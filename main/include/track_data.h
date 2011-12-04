/* THIS FILE IS GENERATED CODE -- DO NOT EDIT */

#pragma once

#include "track_node.h"
#include <lookup.h>

// The track initialization functions expect an array of this size.
#define TRACK_MAX 332

int sensor_mapper(void* key);
lookup* init_tracka(track_node *track);
lookup* init_trackb(track_node *track);
