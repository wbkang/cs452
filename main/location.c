#include <location.h>
#include <util.h>

location location_new(track_edge *edge, fixed offset) {
	ASSERT(edge || fixed_sgn(offset) == 0, "bad location");
	location rv = {edge, fixed_new(0)};
	location_inc(&rv, offset);
	return rv;
}

location location_undef() {
	return location_new(NULL, fixed_new(0));
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
// @TODO: add support for negative dx
int location_inc(location *this, fixed dx) {
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
