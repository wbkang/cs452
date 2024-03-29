#pragma once

#include <track_node.h>

typedef struct location location;

struct location {
	track_edge *edge;
	float offset;
};

location location_fromedge(track_edge *edge);
location location_fromnode(track_node *node, int edge_idx);
location location_undef();
int location_isundef(location *this);
int location_isinvalid(location *this);
int location_dist_dir(location *A, location *B);
int location_dist_min(location *A, location *B);
// int location_dist_abs(location *A, location *B);
int location_add(location *this, float dx);
int location_reverse(location *this);
void location_tonextnode(location *this);
int location_tostring(location *this, char *buf);
