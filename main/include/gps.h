#pragma once

#include <train.h>
#include <location.h>
#include <heap.h>
#include <track_node.h>
#include <ui/logdisplay.h>

struct train;
typedef struct gps gps;
struct gps {
	track_node *track_node;
	heap *heap_dijkstra;
};

gps *gps_new(track_node *nodes);
void dijkstra(track_node *nodeary,
		heap *unoptimized,
		track_node *src,
		track_node *tgt,
		track_node **rv_nodes,
		int *rv_nodecnt,
		struct train *train);
void gps_findpath(gps *this,
		struct train *train,
		location *dest,
		int maxlen,
		trainvcmd *rv_vcmd,
		int *rv_len,
		logdisplay *log);
//void gps_test(gps *this, engineer *eng);
int vcmd2str(char *buf, trainvcmd *vcmd);
