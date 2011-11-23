#include <location.h>
#include <util.h>

location location_new(track_edge *edge) {
	return (location) {edge, fixed_new(0)};
}

location location_undef() {
	return location_new(NULL);
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
	this->offset = fixed_add(this->offset, dx);
	if (sgn > 0) {
		for (;;) {
			fixed len_edge = fixed_new(this->edge->dist);
			if (fixed_cmp(this->offset, len_edge) < 0) return 0;
			track_node *n_next = this->edge->dest;
			ASSERTNOTNULL(n_next);
			track_edge *next_edge = track_next_edge(n_next);
			if (next_edge) {
				this->edge = next_edge;
				this->offset = fixed_sub(this->offset, len_edge);
			} else if (n_next->type == NODE_EXIT) {
				this->offset = fixed_sub(len_edge, fixed_new(1));
				return -2; // dead end
			} else {
				ASSERT(0, "ran into a non-exit node with no edge %s, dir %d", n_next->name, n_next->switch_dir);
				return -3;
			}
		}
	} else {
		// @TODO: problem if backup into an enter
		while (fixed_sgn(this->offset) < 0) {
			track_node *n = this->edge->src;
			track_node *n_rev = n->reverse;
			track_node *n_rev_prev = track_next_node(n_rev);
			if (!n_rev_prev) return -4;
			track_node *n_prev = n_rev_prev->reverse;
			track_edge *edge = n_prev->edge;
			if (edge[0].dest == n) {
				this->edge = &edge[0];
			} else if (edge[1].dest == n) {
				this->edge = &edge[1];
			} else {
				ASSERT(0, "3 edges, wtf?");
			}
			this->offset = fixed_add(fixed_new(this->edge->dist), this->offset);
		}
		return 0;
	}
	*this = location_undef();
	return -5;
}

int location_tostring(location *this, char *buf) {
	char * const origbuf = buf;
	if (location_isundef(this)) {
		buf += sprintf(buf, "(undefined)");
	} else if (fixed_sgn(this->offset) == 0) {
		buf += sprintf(buf, "%s", this->edge->src->name);
	} else {
		buf += sprintf(buf,
			"(%s->%s + %Fmm)",
			this->edge->src->name, this->edge->dest->name, this->offset
		);
	}
	return buf - origbuf;
}

int location_reverse(location *this) {
	if (location_isundef(this)) return -1; // reversing undefined location
	fixed offset = fixed_sub(fixed_new(this->edge->dist), this->offset);
	*this = location_new(this->edge->reverse);
	location_add(this, offset);
	return 0;
}
