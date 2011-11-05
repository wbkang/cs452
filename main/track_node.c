#include <track_node.h>

track_edge * find_forward(track_node *orig) {
	switch (orig->type) {
		case NODE_SENSOR:
		case NODE_MERGE: {
			return &orig->edge[0];
		}
		case NODE_BRANCH: {
			if (orig->switch_dir != -1) {
				return &orig->edge[orig->switch_dir];
			} else {
				return 0;
			}
		}
		default:
			return 0;
	}
}

int find_dist(track_node *orig, track_node *dest, int curdist, int maxsensordepth) {
	//	NODE_SENSOR, NODE_BRANCH, NODE_MERGE, NODE_ENTER, NODE_EXIT
	if (dest == orig) {
		return curdist;
	}

	if (maxsensordepth == -1) {
		return -1;
	}

	switch (orig->type) {
		case NODE_SENSOR: {
			return find_dist(orig->edge[0].dest, dest, curdist + orig->edge[0].dist, maxsensordepth - 1);
		}
		case NODE_BRANCH: {
			int dir = orig->switch_dir;
			if (dir == -1) return -1;
			int dist = find_dist(orig->edge[dir].dest, dest, curdist + orig->edge[dir].dist, maxsensordepth);
			return dist;
		}
		case NODE_MERGE: {
			return find_dist(orig->edge[0].dest, dest, curdist + orig->edge[0].dist, maxsensordepth);
		}
		default:
			return -1;
	}
}
