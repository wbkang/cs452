#include <location.h>
#include <util.h>
#include <stdio.h>

// @TODO: currently we consider exits to be 2cm closer than they are, why not edit the track data?
#define LOCATION_EXIT_MARGIN 20

location location_fromedge(track_edge *edge) {
	return (location) {edge, 0};
}

location location_fromnode(track_node *node, int edge_idx) {
	ASSERTNOTNULL(node);
	ASSERT(edge_idx == 0 || edge_idx == 1, "invalid edge index %d", edge_idx);
	switch (node->type) {
		case NODE_SENSOR:
		case NODE_MERGE:
		case NODE_ENTER:
			return location_fromedge(&node->edge[DIR_AHEAD]);
		case NODE_BRANCH:
			return location_fromedge(&node->edge[edge_idx]);
		case NODE_EXIT: {
			location rv = location_fromnode(node->reverse, DIR_AHEAD);
			location_add(&rv, LOCATION_EXIT_MARGIN);
			location_reverse(&rv);
			return rv;
		}
		case NODE_NONE:
			break;
	}
	ASSERT(0, "node type %d is not handled", node->type);
	return location_undef(); // unreachable
}

location location_undef() {
	return location_fromedge(NULL);
}

int location_isundef(location *this) {
	ASSERTNOTNULL(this);
	return this->edge == NULL && this->offset == 0;
}

int location_isinvalid(location *this) {
	if (!this) return -1;
	if (!this->edge) {
		if (this->offset == 0) return 0; // undefined
		return -2;
	}
	if (this->offset < 0) return -3; // negative offset
	float edge_len = this->edge->dist;
	if (this->offset >= edge_len) return -4; // "overflow" offset
	return 0;
}

// @TODO: virtually in all cases we do "if dist < limit", add a max length to short-circuit?
int location_dist_dir(location *A, location *B) {
	ASSERT(!location_isinvalid(A), "bad 'from' location %d", location_isinvalid(A));
	ASSERT(!location_isinvalid(B), "bad 'to' location %d", location_isinvalid(B));
	if (location_isundef(A)) return -1;
	if (location_isundef(B)) return -2;
	int doff = B->offset - A->offset;
	if (A->edge == B->edge && doff < 0) {
		// A & B share edge but A->offset > B->offset, loop around
		track_node *from_next_node = track_next_node(A->edge->src);
		ASSERTNOTNULL(from_next_node);
		ASSERT(from_next_node != A->edge->src, "same node");
		int dist = track_distance(from_next_node, B->edge->src);
		if (dist < 0) return -3;
		track_edge *next_edge = A->edge;
		ASSERTNOTNULL(next_edge);
		return dist + next_edge->dist + doff;
	} else {
		int dist = track_distance(A->edge->src, B->edge->src);
		if (dist < 0) return -4;
		return dist + doff;
	}
}

int location_dist_min(location *A, location *B) {
	int distAB = location_dist_dir(A, B);
	int distBA = location_dist_dir(B, A);
	int goodAB = distAB >= 0;
	int goodBA = distBA >= 0;
	if (goodAB && goodBA) return min(distAB, distBA);
	if (goodAB) return distAB;
	if (goodBA) return distBA;
	return -1;
}

// int location_dist_abs(location *A, location *B) { // super terrible, dont use
// 	location a = *A;
// 	location_reverse(&a);
// 	location b = *B;
// 	location_reverse(&b);
// 	return min(location_dist_min(A, B), location_dist_min(&a, &b));
// }

// @TODO: don't use current switch state
int location_add(location *this, float dx) {
	ASSERT(!location_isinvalid(this), "bad location %d", location_isinvalid(this));
	if (location_isundef(this)) return -1; // incrementing undefined location
	if (dx == 0) return 0;
	if (dx > 0) {
		int num_sensors = 0;
		this->offset += dx;
		for (;;) {
			float len_edge = this->edge->dist;
			if (this->offset < len_edge) return num_sensors;
			track_node *n_next = this->edge->dest;
			ASSERTNOTNULL(n_next);
			if (n_next->type == NODE_EXIT) {
				*this = location_fromnode(n_next, DIR_AHEAD);
				return -2; // dead end
			} else {
				if (n_next->type == NODE_SENSOR && n_next->name[0] != 'V') {
					num_sensors += 1;
				}
				track_edge *next_edge = track_next_edge(n_next);
				if (next_edge) {
					this->edge = next_edge;
					this->offset -= len_edge;
				} else {
					ASSERT(0, "ran into a non-exit node (%s) with unknown edge dir (%d)", n_next->name, n_next->switch_dir);
					return -3;
				}
			}
		}
	} else {
		location_reverse(this);
		int num_sensors = location_add(this, -dx);
		location_reverse(this);
		return num_sensors;
	}
	*this = location_undef();
	return -5;
}

void location_tonextnode(location *this) {
	ASSERTNOTNULL(this->edge->dest);
	this->edge = track_next_edge(this->edge->dest);
	this->offset = 0;
}

int location_reverse(location *this) {
	if (location_isundef(this)) return -1; // reversing undefined location
	float offset = this->edge->dist - this->offset;
	*this = location_fromedge(this->edge->reverse);
	location_add(this, offset);
	return 0;
}

int location_tostring(location *this, char *buf) {
	char * const origbuf = buf;
	int errno = location_isinvalid(this);
	if (errno) {
		buf += sprintf(buf, "(invalid: %d)", errno);
	} else if (location_isundef(this)) {
		buf += sprintf(buf, "(undefined)");
	} else {
		int dist = this->offset;
		track_node *node = this->edge->src->reverse;
		dist += track_skipvnodes(&node);
		node = node->reverse;
		if (dist == 0) {
			buf += sprintf(buf, "%s", node->name);
		} else {
			buf += sprintf(buf, "%s+%dmm", node->name, dist);
			if (node->type == NODE_BRANCH) {
				buf += sprintf(buf, "->%s", track_next_node(node)->name);
			}
		}
	}
	return buf - origbuf;
}
