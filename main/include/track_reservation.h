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
int reservation_checkpath(reservation_req *req, int train_no);
void reservation_path(reservation_req *req, int train_no);
void reservation_free(reservation_req *req, int train_no);
