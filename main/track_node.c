#include <track_node.h>
#include <constants.h>

track_node *find_next_sensor(track_node *orig) {
	track_edge *edge = find_forward(orig);
	track_node *node;
	while (edge) {
		node = edge->dest;
		if (node->type == NODE_SENSOR) {
			return node;
		}
		edge = find_forward(node);
	}
	return NULL;
}

track_edge *find_forward(track_node *orig) {
	switch (orig->type) {
		case NODE_SENSOR:
		case NODE_MERGE:
			return &orig->edge[0];
		case NODE_BRANCH:
			if (orig->switch_dir == -1) return NULL;
			return &orig->edge[orig->switch_dir];
		default:
			return NULL;
	}
}

int find_dist(track_node *orig, track_node *dest, int curdist, int maxsensordepth) {
	//	NODE_SENSOR, NODE_BRANCH, NODE_MERGE, NODE_ENTER, NODE_EXIT
	if (!orig || !dest) return -1;
	if (dest == orig) return curdist;
	if (maxsensordepth == -1) return -1;

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

int find_path_blind(track_node *orig, track_node *dest, blind_path_result *rv, int maxsensordepth) {
	//	NODE_SENSOR, NODE_BRANCH, NODE_MERGE, NODE_ENTER, NODE_EXIT
	if (dest == orig) return 0;

	if (maxsensordepth == -1) return -1;

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

int calc_distance_after(track_node *orig, int tick_diff, int tref) {
	track_edge *expected_edge = find_forward(orig);
	if (!expected_edge) return -1;
	// TOOD reject big tick_diff values
	fixed beta = expected_edge->beta;
	ASSERT(tick_diff > 0, "tick_diff is nonpositive");
	int dist = expected_edge->dist;
	int tseg = fixed_int(fixed_mul(fixed_new(tref), beta));
	int est_dist_mm = (dist * tick_diff) / tseg;
	return est_dist_mm / 10;
}

fixed beta_sum(track_node *orig, track_node *dest) {
	ASSERTNOTNULL(orig);
	ASSERTNOTNULL(dest);
	ASSERT(orig != dest, "orig and dest are the same. orig:%s, dest: %s", orig->name, dest->name);

	track_node *curnode = orig;
	track_edge *curedge = find_forward(curnode);
	ASSERTNOTNULL(curedge);

	fixed total_beta = fixed_new(0);

	while (curnode != dest) {
		ASSERT(curedge,
				"curedge is null. finding %s to %s, curnode:%s total_beta: %F", orig->name, dest->name, curnode->name, total_beta);
		ASSERT(curedge->beta != fixed_new(-1),
				"edge %s->%s beta is uninitialized.", PREV_EDGE(curedge)->name, curedge->dest->name);
		total_beta = fixed_add(total_beta, curedge->beta);
		curnode = curedge->dest;
		curedge = find_forward(curnode);
	}

	return total_beta;
}
