#include <location.h>
#include <util.h>

void location_init(location *this, track_edge *edge, fixed offset) {
	ASSERTNOTNULL(this);
	this->edge = edge;
	this->offset = fixed_new(0);
	location_inc(this, offset);
}

void location_init_undef(location *this) {
	ASSERTNOTNULL(this);
	this->edge = NULL;
}

int location_isundef(location *this) {
	ASSERTNOTNULL(this);
	return this->edge == NULL;
}

int location_isvalid(location *this) {
	if (this == NULL) return FALSE;
	if (this->edge == NULL) return TRUE; // undefined
	if (fixed_sgn(this->offset) < 0) return FALSE; // negative offset
	fixed edge_len = fixed_new(this->edge->dist);
	if (fixed_cmp(this->offset, edge_len) >= 0) return FALSE; // "overflow" offset
	return TRUE;
}

// @TODO: this works right now only if from->edge->src <= to->edge->src, what if not? perhaps we should do the distance from->to and to->from, and take the smallest valid one?
fixed location_dist(location *from, location *to) {
	ASSERT(location_isvalid(from), "bad 'from' location");
	ASSERT(location_isvalid(to), "bad 'to' location");
	if (!location_isundef(from) && !location_isundef(to)) {
		int dist = track_distance(from->edge->src, to->edge->src);
		if (dist >= 0) {
			fixed rv = fixed_sub(fixed_add(fixed_new(dist), to->offset), from->offset);
			return fixed_abs(rv);
		}
	}
	fixed rv = {-1};
	return rv;
}

// @TODO: problem here if slightly over-increment past an exit/enter
// @TODO: uses current switch state. return multiple 'virtual' locations instead?
void location_inc(location *this, fixed dx) {
	ASSERT(location_isvalid(this), "bad location");
	ASSERT(!location_isundef(this), "undefined locations");
	ASSERT(fixed_sgn(dx) >= 0, "negative dx"); // @TODO: add support for decrementing
	this->offset = fixed_add(this->offset, dx);
	do {
		fixed edge_len = fixed_new(this->edge->dist);
		if (fixed_cmp(this->offset, edge_len) < 0) return;
		this->offset = fixed_sub(this->offset, edge_len);
		this->edge = track_next_edge(this->edge->dest);
	} while (this->edge);
	location_init_undef(this);
}
