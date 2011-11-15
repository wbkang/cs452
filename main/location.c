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

fixed location_dist_min(location *from, location *to) {
	ASSERT(location_isvalid(from), "bad 'from' location");
	ASSERT(location_isvalid(to), "bad 'to' location");
	if (!location_isundef(from) && !location_isundef(to)) {
		fixed dista = fixed_new(track_distance(from->edge->src, to->edge->src));
		fixed distb = fixed_new(track_distance(to->edge->src, from->edge->src));
		fixed doff = fixed_sub(to->offset, from->offset);
		int gooda = fixed_sgn(dista) >= 0;
		int goodb = fixed_sgn(distb) >= 0;
		if (gooda && goodb) {
			fixed a = fixed_abs(fixed_add(dista, doff));
			fixed b = fixed_abs(fixed_sub(distb, doff));
			return fixed_min(a, b);
		} else if (gooda) {
			return fixed_abs(fixed_add(dista, doff));
		} else if (goodb) {
			return fixed_abs(fixed_sub(distb, doff));
		}
	}
	fixed rv = {-1};
	return rv;
}

fixed location_dist_dir(location *from, location *to) {
	ASSERT(location_isvalid(from), "bad 'from' location");
	ASSERT(location_isvalid(to), "bad 'to' location");
	if (!location_isundef(from) && !location_isundef(to)) {
		fixed doff = fixed_sub(to->offset, from->offset);
		// loop around
		if (from->edge == to->edge && fixed_cmp(from->offset, to->offset) > 0) {
			track_node *from_next_node = track_next_node(from->edge->src);
			ASSERTNOTNULL(from_next_node);
			fixed dist = fixed_new(track_distance(from_next_node, from->edge->src));
			ASSERT(from_next_node != from->edge->src, "same node");
			if (fixed_sgn(dist) > 0) {
				track_edge *next_edge = from->edge;
				ASSERTNOTNULL(next_edge);
				return fixed_sub(fixed_add(dist, fixed_new(next_edge->dist)), doff);
			}
		} else {
			fixed dist = fixed_new(track_distance(from->edge->src, to->edge->src));
			if (fixed_sgn(dist) >= 0) {
				return fixed_add(dist, doff);
			}
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
