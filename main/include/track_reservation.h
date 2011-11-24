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

int reserve_checkpath(int trainno, reservation_req *req);
void reserve_path(int trainno, reservation_req *req);
void reserve_return(int trainno, reservation_req *req);
