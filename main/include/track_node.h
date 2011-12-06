#pragma once

typedef enum {
	NODE_NONE, NODE_SENSOR, NODE_BRANCH, NODE_MERGE, NODE_ENTER, NODE_EXIT
} node_type;

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

#define TRAIN_UNOCCUPIED -1

typedef struct track_node track_node;
typedef struct track_edge track_edge;

struct track_edge {
	track_edge *reverse;
	track_node *src, *dest;
	int owner;
	int dist; // in mm
	int num;
};

struct track_node {
	const char *name;
	node_type type;
	int num; // sensor or switch number
	track_node *reverse; // same location, but opposite direction
	track_edge edge[2];
	int switch_dir;
};

#define POS2DIR(pos) (track_switchpos_curved(pos) ? DIR_CURVED : DIR_STRAIGHT)
#define PREV_NODE(edge) (edge->reverse->dest->reverse)

static inline int track_switchpos_straight(int pos) {
	return pos == 's' || pos == 'S';
}

static inline int track_switchpos_curved(int pos) {
	return pos == 'c' || pos == 'C';
}

static inline int track_switchpos_isgood(int pos) {
	return track_switchpos_straight(pos) || track_switchpos_curved(pos);
}

track_edge *track_get_edge(track_node *from, track_node *to);
track_edge *track_next_edge(track_node *node);
track_node *track_next_node(track_node *node);
int track_distance(track_node *from, track_node *to);
int track_numedges(track_node *node);
int track_walk(track_node *node, int dist, int maxlen, track_edge *arr[], int *len);
int track_skipvnodes(track_node **node);
char track_branch_edge2pos(track_node *br, track_edge *edge);
