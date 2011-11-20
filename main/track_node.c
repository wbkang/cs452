#include <track_node.h>
#include <constants.h>

track_edge *track_next_edge(track_node *node) {
	if (!node) return NULL; // bad node
	switch (node->type) {
		case NODE_SENSOR:
		case NODE_ENTER:
		case NODE_MERGE:
			return &node->edge[0];
		case NODE_BRANCH: {
			int dir = node->switch_dir;
			if (dir < 0) return NULL; // unknown branch position
			return &node->edge[dir];
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
	if (!from || !to) return -1; // bad nodes

	track_node *jump1 = from;
	track_node *jump2 = from;
	int dist = 0;

	while (jump1 != to) {
		track_edge *edge = track_next_edge(jump1);
		if (!edge) return -2; // no more edges
		dist += edge->dist;
		jump1 = edge->dest;
		if (!jump1) return -3; // unending edge
		jump2 = track_next_node(track_next_node(jump2));
		if (jump1 == jump2) return -4; // cycle
	}

	return dist;
}
