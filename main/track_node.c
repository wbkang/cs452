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
		default:
			return NULL; // unkown node type
	}
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
		default:
			ASSERT(0, "node type %d is not handled", node->type);
			return 0; // unreachable
	}
}

// starting from 'node' walk a distance 'dist' forward and put all met edges into 'arr'
int track_walk(track_node *node, int dist, int maxlen, track_edge *arr[], int *len) {
	if (dist <= 0) return TRUE;
	if (!node) return FALSE;

	int num_edges = track_numedges(node);
	if (num_edges == 0) return node->type == NODE_EXIT; // exits are a special case

	for (int i = 0; i < num_edges; i++) {
		track_edge *edge = &node->edge[i];
		ASSERT(*len < maxlen, "'arr' full, len %d max len %d", *len, maxlen);
		arr[(*len)++] = edge;
		if (!track_walk(edge->dest, dist - edge->dist, maxlen, arr, len)) return FALSE;
	}

	return TRUE;
}
