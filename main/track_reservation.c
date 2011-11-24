#include <track_reservation.h>
#include <util.h>
#include <syscall.h>
#include <train.h>

static inline void mark_occupied(int trainno, track_edge *e) {
	ASSERTNOTNULL(e);
	ASSERT(e->owner == trainno || e->owner == TRAIN_UNOCCUPIED,
			"tr%d trying to occupy the edge %s->%s owned by %d.",
			trainno, e->src->name, e->dest->name, e->owner);
	e->owner = trainno;
	e->reverse->owner = trainno;
}

static inline void mark_free(int trainno, track_edge *e) {
	ASSERTNOTNULL(e);
	ASSERT(e->owner == TRAIN_UNOCCUPIED || e->owner == trainno,
			"train edge %s->%s already occupied by tr%d",
			e->src->name, e->dest->name, e->owner);
	ASSERT(e->reverse->owner == TRAIN_UNOCCUPIED || e->reverse->owner == trainno,
				"train edge %s->%s already occupied by tr%d",
				e->src->name, e->dest->name, e->owner);
	e->owner = TRAIN_UNOCCUPIED;
	e->reverse->owner = TRAIN_UNOCCUPIED;
}

static inline int can_occupy(int trainno, track_edge *e) {
	ASSERTNOTNULL(e);
//	ASSERT(e->owner == trainno || e->owner == TRAIN_UNOCCUPIED,
//				"tr%d trying to occupy the edge %s->%s owned by %d.",
//				trainno, e->src->name, e->dest->name, e->owner);
	return (e->owner == trainno || e->owner == TRAIN_UNOCCUPIED) &&
			(e->reverse->owner == trainno || e->reverse->owner == TRAIN_UNOCCUPIED);
}

int reserve_checkpath(int trainno, reservation_req *req) {
	ASSERTNOTNULL(req);
	for (int i = 0; i < req->len; i++) {
		ASSERTNOTNULL(req->edges[i]);
		if (!can_occupy(trainno, req->edges[i])) {
			return FALSE;
		}
	}
	return TRUE;
}

void reserve_path(int trainno, reservation_req *req) {
	ASSERTNOTNULL(req);
	ASSERT(reserve_checkpath(trainno, req), "can't reserve the path for train %d", trainno);
	for (int i = 0; i < req->len; i++) {
		ASSERTNOTNULL(req->edges[i]);
		mark_occupied(trainno, req->edges[i]);
	}
}

void reserve_return(int trainno, reservation_req *req) {
	ASSERTNOTNULL(req);
	for (int i = 0; i < req->len; i++) {
		ASSERTNOTNULL(req->edges[i]);
		mark_free(trainno, req->edges[i]);
	}
}
