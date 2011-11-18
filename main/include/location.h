#pragma once

#include <track_node.h>

typedef struct location location;

struct location {
	track_edge *edge;
	fixed offset;
};

#define LOCATION_UNDEF (location_new(NULL, fixed_new(0)))

location location_new(track_edge *edge, fixed offset);
int location_isundef(location *this);
int location_isvalid(location *this);
fixed location_dist_min(location *from, location *to);
fixed location_dist_dir(location *from, location *to);
int location_inc(location *this, fixed dx);
