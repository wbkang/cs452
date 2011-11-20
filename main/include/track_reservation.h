#pragma once

#include <track_node.h>

#define TRAIN_UNOCCUPIED -1
#define NUM_EDGES 156 // hardcoded

typedef struct track_reservation track_reservation;

struct track_reservation {
	struct edge_occupation {
		int train_no; // -1 if unoccupied
	} reservation[NUM_EDGES];
};

typedef struct path path;

struct path {
	int len;
	track_edge edges[];
};

track_reservation *track_reservation_new();
int track_reservation_canreserve(track_reservation *this, path *path);
int track_reservation_reserve(track_reservation *this, int train_no, path *path);
