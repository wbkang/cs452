#pragma once

#include <engineer.h>
#include <train.h>
#include <location.h>
#include <heap.h>
#include <track_node.h>

typedef struct gps gps;

struct gps {
	track_node *track_node;
	heap *heap_dijkstra;
};

gps *gps_new(track_node *nodes);
void gps_findpath(gps *this, train_descriptor *train, location *dest, int maxlen, trainvcmd *rv_vcmd[], int *rv_len);
void gps_test(gps *this);
