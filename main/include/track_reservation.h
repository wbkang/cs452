#pragma once

#include <track_node.h>
#include <location.h>
#include <gps.h>

typedef struct path path;

struct path {
	location start;
	location end;
	int pathlen;
	track_node *nodes[MAX_PATH];
};

typedef struct reservation_req reservation_req;

struct reservation_req {
	track_edge *edges[MAX_PATH];
	int len;
};

int can_occupy(track_edge *e, int train_no);
int reservation_replace(reservation_req *req_old, reservation_req *req_new, int train_no);
