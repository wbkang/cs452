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

int reserve_checkpath(int trainno, path *p);
int reserve_path(int trainno, path *p);
int reserve_return(int trainno, path *p);
