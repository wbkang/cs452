#pragma once

#include <track_node.h>
#include <location.h>
#include <gps.h>
#include <track_data.h>

typedef struct path path;

struct path {
	location start;
	location end;
	int pathlen;
	track_node *nodes[TRACK_MAX];
};

typedef struct reservation_req reservation_req;

struct reservation_req {
	track_edge *edges[TRACK_MAX];
	int len;
};

int can_occupy(track_edge *e, int train_no);
int reservation_replace(reservation_req *req_old, reservation_req *req_new, int train_no);
