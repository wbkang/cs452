#include <track_node.h>
#include <constants.h>
#include <util.h>

track_edge *track_get_edge(track_node *from, track_node *to) {
	for (int i = track_numedges(from) - 1; i >= 0; --i) {
		track_edge *edge = &from->edge[i];
		if (edge->dest == to) return edge;
	}
	return NULL;
}

track_edge *track_next_edge(track_node *node) {
	if (!node) return NULL; // bad node
	switch (node->type) {
		case NODE_SENSOR:
		case NODE_ENTER:
		case NODE_MERGE:
			return &node->edge[0];
		case NODE_BRANCH: {
			int dir = node->switch_dir;
			if (dir == 0 || dir == 1) return &node->edge[dir];
			return NULL; // unknown branch position
		}
		case NODE_NONE:
		case NODE_EXIT:
			return NULL;
	}
	return NULL;
}

track_node *track_next_node(track_node *node) {
	track_edge *edge = track_next_edge(node);
	if (!edge) return NULL;
	return edge->dest;
}

int track_distance(track_node *from, track_node *to) {
	if (!from) return -1; // bad 'from' node
	if (!to) return -2; // bad 'to' node

	track_node *jump1 = from;
	track_node *jump2 = from;
	int dist = 0;

	while (jump1 != to) {
		track_edge *edge = track_next_edge(jump1);
		if (!edge) return -3; // no more edges
		dist += edge->dist;
		jump1 = edge->dest;
		if (!jump1) return -4; // unending edge
		jump2 = track_next_node(track_next_node(jump2));
		if (jump1 == jump2) return -5; // cycle
	}

	return dist;
}

int track_numedges(track_node *node) {
	ASSERTNOTNULL(node);
	switch (node->type) {
		case NODE_BRANCH:
			return 2;
		case NODE_SENSOR:
		case NODE_MERGE:
		case NODE_ENTER:
			return 1;
		case NODE_NONE:
		case NODE_EXIT:
			return 0;
	}
	ASSERT(0, "node type %d is not handled", node->type);
	return 0; // unreachable
}

// starting from 'node' walk a distance 'dist' forward and put all met edges into 'arr'
// int track_walk(track_node *node, int dist, int maxlen, track_edge *arr[], int *len) {
// 	while (dist > 0) {
// 		if (!node) return FALSE;

// 		int num_edges = track_numedges(node);

// 		if (num_edges == 0) return node->type == NODE_EXIT; // exits are a special case

// 		if (num_edges == 2) {
// 			track_edge *edge = &node->edge[1];
// 			if (!in_array((void**) arr, *len, edge) && !in_array((void**) arr, *len, edge->reverse)) {
// 				ASSERT(*len < maxlen, "'arr' full, len %d max len %d", *len, maxlen);
// 				arr[(*len)++] = edge;
// 			}
// 			if (!track_walk(edge->dest, dist - edge->dist, maxlen, arr, len)) return FALSE;
// 		}

// 		if (num_edges >= 1) {
// 			track_edge *edge = &node->edge[0];
// 			if (!in_array((void**) arr, *len, edge) && !in_array((void**) arr, *len, edge->reverse)) {
// 				ASSERT(*len < maxlen, "'arr' full, len %d max len %d", *len, maxlen);
// 				arr[(*len)++] = edge;
// 			}
// 			node = edge->dest;
// 			dist -= edge->dist;
// 		}
// 	}
// 	return TRUE;
// }

// starting from 'node' walk a distance 'dist' forward and put all met edges into 'arr'
int track_walk(track_node *node, int dist, int maxlen, track_edge *arr[], int *len) {
	if (dist <= 0) return TRUE;
	if (!node) return FALSE;

	int num_edges = track_numedges(node);
	if (num_edges == 0) return node->type == NODE_EXIT; // exits are a special case

	for (int i = 0; i < num_edges; i++) {
		track_edge *edge = &node->edge[i];
		ASSERT(*len < maxlen, "'arr' full, len %d max len %d", *len, maxlen);
		// if (*len >= maxlen) return FALSE;
		if (!in_array((void**) arr, *len, edge) && !in_array((void**) arr, *len, edge->reverse)) {
			arr[(*len)++] = edge;
		}
		if (!track_walk(edge->dest, dist - edge->dist, maxlen, arr, len)) return FALSE;
	}

	return TRUE;
}

int track_skipvnodes(track_node **node) {
	if (!node) return -1; // NULL
	int dist = 0;
	for (;;) {
		if (!*node) return -2; // NULL
		if ((*node)->name[0] != 'V') return dist;
		track_edge *edge = track_next_edge(*node);
		dist += edge->dist;
		*node = edge->dest;
	}
	return -3; // unreachable
}

char track_branch_edge2pos(track_node *br, track_edge *edge) {
	if (edge == &br->edge[DIR_STRAIGHT]) return 's';
	if (edge == &br->edge[DIR_CURVED]) return 'c';
	ASSERT(0, "'edge' is not a vaid edge of 'br'");
	return '\0'; // unreachable
}
