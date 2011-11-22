#pragma once

#include <track_node.h>
#include <fixed.h>

typedef struct location location;

struct location {
	track_edge *edge;
	fixed offset;
};

location location_new(track_edge *edge);
location location_undef();
int location_isundef(location *this);
int location_isvalid(location *this);
int location_dist_min(location *A, location *B);
int location_dist_dir(location *A, location *B);
int location_add(location *this, fixed dx);
int location_tostring(location *this, char *buf);
int location_reverse(location *this);
