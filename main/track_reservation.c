#include <track_reservation.h>
#include <util.h>
#include <syscall.h>
#include <train.h>

// @TODO: implement sparse set to improve this
// http://research.swtch.com/2008/03/using-uninitialized-memory-for-fun-and.html

int can_occupy(track_edge *e, int train_no) {
	ASSERTNOTNULL(e);
//	ASSERT(e->owner == train_no || e->owner == TRAIN_UNOCCUPIED,
//				"tr%d trying to occupy the edge %s->%s owned by %d.",
//				train_no, e->src->name, e->dest->name, e->owner);
	return (e->owner == train_no || e->owner == TRAIN_UNOCCUPIED) &&
			(e->reverse->owner == train_no || e->reverse->owner == TRAIN_UNOCCUPIED);
}

static inline void mark_occupied(track_edge *e, int train_no) {
	ASSERT(can_occupy(e, train_no),
			"train %d trying to occupy edge %s->%s owned by %d.",
			train_no, e->src->name, e->dest->name, e->owner);
	e->owner = train_no;
	e->reverse->owner = train_no;
}

static inline void mark_free(track_edge *e, int train_no) {
	ASSERT(can_occupy(e, train_no),
			"edge %s->%s already occupied by train %d",
			e->src->name, e->dest->name, e->owner);
	e->owner = TRAIN_UNOCCUPIED;
	e->reverse->owner = TRAIN_UNOCCUPIED;
}

static int reservation_checkpath(reservation_req *req, int train_no) {
	ASSERTNOTNULL(req);
	for (int i = 0; i < req->len; i++) {
		if (!can_occupy(req->edges[i], train_no)) return FALSE;
	}
	return TRUE;
}

static void reservation_path(reservation_req *req, int train_no) {
	ASSERTNOTNULL(req);
	for (int i = 0; i < req->len; i++) {
		mark_occupied(req->edges[i], train_no);
	}
}

static void reservation_free(reservation_req *req, int train_no) {
	ASSERTNOTNULL(req);
	for (int i = 0; i < req->len; i++) {
		mark_free(req->edges[i], train_no);
	}
	req->len = 0;
}

int reservation_replace(reservation_req *req_old, reservation_req *req_new, int train_no) {
	if (!reservation_checkpath(req_new, train_no)) return FALSE;
	reservation_free(req_old, train_no);
	reservation_path(req_new, train_no);
	*req_old = *req_new;
	return TRUE;
}
