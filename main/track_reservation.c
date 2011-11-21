#include <track_reservation.h>
#include <util.h>
#include <syscall.h>

track_reservation *track_reservation_new() {
	track_reservation *this = malloc(sizeof(track_reservation));
	for (int i = 0; i < NUM_EDGES; i++) {
		this->reservation[i].train_no = TRAIN_UNOCCUPIED;
	}
	return this;
}

int track_reservation_canreserve(track_reservation *this, path *path) {
	for (int i = 0; i < path->len; i++) {
		int edgenum = path->edges[i].num;
		int reversenum = path->edges[i].reverse->num;
		if (this->reservation[edgenum].train_no != TRAIN_UNOCCUPIED
				|| this->reservation[reversenum].train_no != TRAIN_UNOCCUPIED) {
			return FALSE;
		}
	}
	return TRUE;
}

int track_reservation_reserve(track_reservation *this, int train_no, path *path) {
	if (!track_reservation_canreserve(this, path)) return FALSE;
	for (int i = 0; i < path->len; i++) {
		int edgenum = path->edges[i].num;
		int reversenum = path->edges[i].reverse->num;
		this->reservation[edgenum].train_no = train_no;
		this->reservation[reversenum].train_no = train_no;
	}
	return TRUE;
}
