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

int location_isvalid(location *this) {
	if (this == NULL) return FALSE;
	if (this->edge == NULL) return fixed_sgn(this->offset) == 0; // undefined
	if (fixed_sgn(this->offset) < 0) return FALSE; // negative offset
	fixed edge_len = fixed_new(this->edge->dist);
	if (fixed_cmp(this->offset, edge_len) >= 0) return FALSE; // "overflow" offset
	return TRUE;
}

int location_dist_min(location *A, location *B) {
	ASSERT(location_isvalid(A), "bad 'from' location");
	ASSERT(location_isvalid(B), "bad 'to' location");
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
	ASSERT(location_isvalid(A), "bad 'from' location");
	ASSERT(location_isvalid(B), "bad 'to' location");
	if (location_isundef(A) || location_isundef(B)) -1;
	int doff = fixed_int(fixed_sub(B->offset, A->offset));
	if (A->edge == B->edge && fixed_cmp(A->offset, B->offset) > 0) {
		// A & B share edge but A->offset > B->offset, loop around
		track_node *from_next_node = track_next_node(A->edge->src);
		ASSERTNOTNULL(from_next_node);
		ASSERT(from_next_node != A->edge->src, "same node");
		int dist = track_distance(from_next_node, A->edge->src);
		if (dist < 0) return -2;
		track_edge *next_edge = A->edge;
		ASSERTNOTNULL(next_edge);
		return dist + next_edge->dist - doff;
	} else {
		int dist = track_distance(A->edge->src, B->edge->src);
		if (dist < 0) return -3;
		return dist + doff;
	}
}

// @TODO: potential problem here if slightly over-increment past an exit/enter
// @TODO: uses current switch state. return multiple 'virtual' locations instead?
int location_add(location *this, fixed dx) {
	ASSERT(location_isvalid(this), "bad location");
	if (location_isundef(this)) return -1; // incrementing undefined location
	int sgn = fixed_sgn(dx);
	if (sgn == 0) return 0;
	this->offset = fixed_add(this->offset, dx);
	if (sgn > 0) {
		for (;;) {
			fixed len_edge = fixed_new(this->edge->dist);
			if (fixed_cmp(this->offset, len_edge) < 0) return 0;
			track_edge *next_edge = track_next_edge(this->edge->dest);
			if (next_edge) {
				this->edge = next_edge;
				this->offset = fixed_sub(this->offset, len_edge);
			} else {
				this->offset = len_edge;
				return -2; // dead end
			}
		}
	} else {
		while (fixed_sgn(this->offset) < 0) {
			track_node *n = this->edge->src;
			n = n->reverse;
			n = track_next_node(n);
			if (!n) return -4;
			n = n->reverse;
			this->edge = n->edge;
			this->offset = fixed_add(this->offset, fixed_new(n->edge->dist));
		}
		return 0;
	}
	*this = location_undef();
	return -3;
}

int location_tostring(location *this, char *buf) {
	char * const origbuf = buf;
	if (location_isundef(this)) {
		buf += sprintf(buf, "[location: UNDEFINED]");
	} else {
		buf += sprintf(buf,
			"[location: %s->%s, offset: %F]",
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
