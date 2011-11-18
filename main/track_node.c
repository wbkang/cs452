#include <track_node.h>
#include <constants.h>
#include <string.h>

// int hash_sensor(char module[], int id) {
// 	ASSERTNOTNULL(module);
// 	int m = 'A' - module[0]; // should be in [0, 4]
// 	ASSERT(0 <= m && m <= 4, "bad module");
// 	int i = id - 1; // should be in [0, 15]
// 	ASSERT(0 <= i && i <= 15, "bad id");
// 	return 5 * i + m;
// }

track_edge *find_forward(track_node *orig) {
	switch (orig->type) {
		case NODE_SENSOR:
		case NODE_MERGE:
			return &orig->edge[0];
		case NODE_BRANCH:
			if (orig->switch_dir < 0) return NULL;
			return &orig->edge[orig->switch_dir];
		default:
			return NULL;
	}
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

int find_path_blind(track_node *orig, track_node *dest, blind_path_result *rv, int maxsensordepth) {
	//	NODE_SENSOR, NODE_BRANCH, NODE_MERGE, NODE_ENTER, NODE_EXIT
	if (dest == orig) return 0;

	if (maxsensordepth < 0) return -1;

	int len;
	switch (orig->type) {
	case NODE_SENSOR: {
		rv->edges[rv->depth++] = &orig->edge[0];
		len = find_path_blind(orig->edge[0].dest, dest, rv, maxsensordepth - 1);
		return (len >= 0) ? len + orig->edge[0].dist : -1;
	}
	case NODE_BRANCH: {
		int depth = rv->depth;

		rv->edges[rv->depth++] = &orig->edge[DIR_STRAIGHT];
		len = find_path_blind(orig->edge[DIR_STRAIGHT].dest, dest, rv, maxsensordepth);
		if (len >= 0) {
			return orig->edge[DIR_STRAIGHT].dist + len;
		}

		rv->depth = depth;
		rv->edges[rv->depth++] = &orig->edge[DIR_CURVED];
		len = find_path_blind(orig->edge[DIR_CURVED].dest, dest, rv, maxsensordepth);
		if (len >= 0) {
			return orig->edge[DIR_CURVED].dist + len;
		}

		return -1;
	}
	case NODE_MERGE: {
		rv->edges[rv->depth++] = &orig->edge[0];
		len = find_path_blind(orig->edge[0].dest, dest, rv, maxsensordepth);
		return (len >= 0) ? len + orig->edge[0].dist : -1;
	}
	default:
		return -1;
	}
}

