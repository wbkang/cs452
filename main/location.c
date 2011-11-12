#include <location.h>

void location_init(location *this, track_edge *edge, fixed offset) {
	this->edge = edge;
	this->offset = offset;
}

void location_init_undef(location *this) {
	this->edge = NULL;
}

int location_isundef(location *this) {
	return !this || this->edge == NULL;
}

fixed location_dist(location *from, location *to) {
	int dist = track_distance(from->edge->src, to->edge->src);
	if (dist < 0) {
		fixed rv = {-1};
		return rv;
	}
	return fixed_sub(fixed_add(fixed_new(dist), from->offset), to->offset);
}

void location_inc(location *this, fixed dx) {
	if (!location_isundef(this)) {
		this->offset = fixed_add(this->offset, dx);
		do {
			fixed fdist = fixed_new(this->edge->dist);
			if (fixed_cmp(this->offset, fdist) < 0) return;
			this->offset = fixed_sub(this->offset, fdist);
			this->edge = track_next_edge(this->edge->dest);
		} while (this->edge);
	}
	location_init_undef(this);
}
