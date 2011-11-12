#pragma once

#include <track_node.h>

typedef struct location location;

struct location {
	track_edge *edge;
	fixed offset;
};

void location_init(location *this, track_edge *edge, fixed offset);
void location_init_undef(location *this);
int location_isundef(location *this);
int location_isvalid(location *this);
fixed location_dist(location *from, location *to);
void location_inc(location *this, fixed dx);
