#pragma once
#include <fixed.h>
#include <train.h>

typedef enum {
	NODE_NONE, NODE_SENSOR, NODE_BRANCH, NODE_MERGE, NODE_ENTER, NODE_EXIT,
} node_type;

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

#define POS2DIR(pos) (train_switchpos_curved(pos) ? DIR_CURVED : DIR_STRAIGHT)

struct track_node;
typedef struct track_node track_node;
typedef struct track_edge track_edge;

struct track_edge {
	track_edge *reverse;
	track_node *src, *dest;
	int dist; /* in millimetres */
	fixed beta; // initialized -1
};

struct track_node {
	const char *name;
	node_type type;
	int num; /* sensor or switch number */
	track_node *reverse; /* same location, but opposite direction */
	track_edge edge[2];
	int switch_dir;
	void* data;
};

track_edge *find_forward(track_node *orig);
int find_dist(track_node *orig, track_node *dest, int curdist, int maxsensordepth);
int calc_distance_after(track_node *orig, int tick_diff, int tref);

typedef struct blind_path_result blind_path_result;
struct blind_path_result {
	int depth;
	track_edge *edges[10];
};

int find_path_blind(track_node *orig, track_node *dest, blind_path_result *rv, int maxsensordepth);
fixed beta_sum(track_node *orig, track_node *dest);
#define PREV_EDGE(edge) (edge->reverse->dest->reverse)
