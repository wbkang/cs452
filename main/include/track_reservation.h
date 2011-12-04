#pragma once

#include <track_node.h>
#include <location.h>
#include <gps.h>
#include <track_data.h>

typedef struct path path;

#define MAX_PATH_LEN (2 * TRACK_MAX)

struct path {
	location start;
	location end;
	int pathlen;
	track_node *nodes[MAX_PATH_LEN];
};

typedef struct reservation_req reservation_req;

struct reservation_req {
	track_edge *edges[MAX_PATH_LEN];
	int len;
};

int can_occupy(track_edge *e, int train_no);
int reservation_replace(reservation_req *req_old, reservation_req *req_new, int train_no);
