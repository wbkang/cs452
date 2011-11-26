#include <location.h>
#include <util.h>

#define LOCATION_EXIT_MARGIN 20

location location_fromedge(track_edge *edge) {
	return (location) {edge, fixed_new(0)};
}

location location_fromnode(track_node *node, int edge_idx) {
	ASSERTNOTNULL(node);
	switch (node->type) {
		case NODE_SENSOR:
		case NODE_MERGE:
		case NODE_ENTER:
			return location_fromedge(&node->edge[DIR_AHEAD]);
		case NODE_BRANCH:
			return location_fromedge(&node->edge[edge_idx]);
		case NODE_EXIT: {
			location rv = location_fromnode(node->reverse, DIR_AHEAD);
			location_add(&rv, fixed_new(LOCATION_EXIT_MARGIN));
			location_reverse(&rv);
			return rv;
		}
		default:
			ASSERT(0, "bad node type %d", node->type);
			return location_undef();
	}
}

location location_undef() {
	return location_fromedge(NULL);
}

int location_isundef(location *this) {
	ASSERTNOTNULL(this);
	return this->edge == NULL && fixed_sgn(this->offset) == 0;
}

int location_isinvalid(location *this) {
	if (this == NULL) return -1;
	if (this->edge == NULL) {
		if (fixed_sgn(this->offset) == 0) return 0; // undefined
		return -2;
	}
	if (fixed_sgn(this->offset) < 0) return -3; // negative offset
	fixed edge_len = fixed_new(this->edge->dist);
	if (fixed_cmp(this->offset, edge_len) >= 0) return -4; // "overflow" offset
	return 0;
}

int location_dist_min(location *A, location *B) {
	ASSERT(!location_isinvalid(A), "bad 'from' location: %d", location_isinvalid(A));
	ASSERT(!location_isinvalid(B), "bad 'to' location: %d", location_isinvalid(B));
	if (location_isundef(A) || location_isundef(B)) return -1;
	int distAB = track_distance(A->edge->src, B->edge->src);
	int distBA = track_distance(B->edge->src, A->edge->src);
	int badAB = distAB < 0;
	int badBA = distBA < 0;
	if (badAB && badBA) return -2;
	int doff = fixed_int(fixed_sub(B->offset, A->offset));
	if (badAB) return abs(distBA - doff);
	if (badBA) return abs(distAB + doff);
	return min(abs(distAB + doff), abs(distBA - doff));
}

int location_dist_abs(location *A, location *B) { // @TODO: this is terrible
	location a = *A;
	location b = *B;
	int dist1 = location_dist_min(A, B);
	location_reverse(&a);
	location_reverse(&b);
	int dist2 = location_dist_min(A, B);
	return min(dist1, dist2);
}

int location_dist_dir(location *A, location *B) {
	ASSERT(!location_isinvalid(A), "bad 'from' location: %d", location_isinvalid(A));
	ASSERT(!location_isinvalid(B), "bad 'to' location: %d", location_isinvalid(B));
	if (location_isundef(A)) return -1;
	if (location_isundef(B)) return -2;
	int doff = fixed_int(fixed_sub(B->offset, A->offset));
	if (A->edge == B->edge && fixed_cmp(A->offset, B->offset) > 0) {
		// A & B share edge but A->offset > B->offset, loop around
		track_node *from_next_node = track_next_node(A->edge->src);
		ASSERTNOTNULL(from_next_node);
		ASSERT(from_next_node != A->edge->src, "same node");
		int dist = track_distance(from_next_node, A->edge->src);
		if (dist < 0) return -3;
		track_edge *next_edge = A->edge;
		ASSERTNOTNULL(next_edge);
		return dist + next_edge->dist - doff;
	} else {
		int dist = track_distance(A->edge->src, B->edge->src);
		if (dist < 0) return -4;
		return dist + doff;
	}
}

// @TODO: uses current switch state. return multiple 'virtual' locations instead?
int location_add(location *this, fixed dx) {
	ASSERT(!location_isinvalid(this), "bad location: %d", location_isinvalid(this));
	if (location_isundef(this)) return -1; // incrementing undefined location
	int sgn = fixed_sgn(dx);
	if (sgn == 0) return 0;
	if (sgn > 0) {
		this->offset = fixed_add(this->offset, dx);
		for (;;) {
			fixed len_edge = fixed_new(this->edge->dist);
			if (fixed_cmp(this->offset, len_edge) < 0) return 0;
			track_node *n_next = this->edge->dest;
			ASSERTNOTNULL(n_next);
			if (n_next->type == NODE_EXIT) {
				*this = location_fromnode(n_next, DIR_AHEAD);
				return -2; // dead end
			} else {
				track_edge *next_edge = track_next_edge(n_next);
				if (next_edge) {
					this->edge = next_edge;
					this->offset = fixed_sub(this->offset, len_edge);
				} else {
					ASSERT(0, "ran into a non-exit node (%s) with unknown edge dir (%d)", n_next->name, n_next->switch_dir);
					return -3;
				}
			}
		}
	} else {
		location_reverse(this);
		location_add(this, fixed_neg(dx));
		location_reverse(this);
		return 0;
	}
	*this = location_undef();
	return -5;
}

void location_tonextnode(location *this) {
	this->edge = track_next_edge(this->edge->dest);
	this->offset = fixed_new(0);
}

int location_reverse(location *this) {
	if (location_isundef(this)) return -1; // reversing undefined location
	fixed offset = fixed_sub(fixed_new(this->edge->dist), this->offset);
	*this = location_fromedge(this->edge->reverse);
	location_add(this, offset);
	return 0;
}

int location_tostring(location *this, char *buf) {
	char * const origbuf = buf;
	if (location_isundef(this)) {
		buf += sprintf(buf, "(undefined)");
	} else if (fixed_sgn(this->offset) == 0) {
		buf += sprintf(buf, "%s", this->edge->src->name);
	} else {
		buf += sprintf(buf,
			"(%s-%s+%dmm)",
			this->edge->src->name, this->edge->dest->name, fixed_int(this->offset)
		);
	}
	return buf - origbuf;
}
