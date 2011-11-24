#pragma once

typedef enum {
	NODE_NONE, NODE_SENSOR, NODE_BRANCH, NODE_MERGE, NODE_ENTER, NODE_EXIT
} node_type;

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1

#define TRAIN_UNOCCUPIED -1
#define MAX_PATH 160

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
	int num; // sensor or switch number @TODO: not needed?
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

static inline track_edge *track_get_edge(track_node *from, track_node *to) {
	if (from->type == NODE_BRANCH && from->edge[1].dest == to) {
		return &from->edge[1];
	} else if (from->edge[0].dest == to) {
		return &from->edge[0];
	} else {
		return 0; // unreachable
	}
}


track_edge *track_next_edge(track_node *node);
track_node *track_next_node(track_node *node);
int track_distance(track_node *from, track_node *to);
