#include <gps.h>
#include <syscall.h>
#include <track_node.h>
#include <track_data.h>
#include <util.h>
#include <heap.h>
#include <string.h>

#define MAX_PATH 160

#define SWITCH_DIST 50

#define CRUISE_SPEED 12

static void dijkstra(gps *this, track_node *src, track_node *tgt, track_edge **rv_edge, int *rv_edgecnt);
static fixed gps_distace(location *start, location *end, track_edge **path, int pathlen);

gps *gps_new(track_node *nodes) {
	gps *p = malloc(sizeof(gps));
	ASSERT(sizeof(gps) > 0, "size of gps is zero");
	ASSERT(p != NULL, "p is null");
	ASSERT(nodes, "size of gps is zero");
	p->track_node = nodes;
	return p;
}

static inline void trainvcmd_addspeed(trainvcmd *rv_vcmd[], int *idx, int speed) {
	rv_vcmd[*idx]->type = TRAINVCMD;
	rv_vcmd[*idx]->name = SETSPEED;
	rv_vcmd[*idx]->data.speed = CRUISE_SPEED;
	(*idx)++;
}

static inline void trainvcmd_addwaitloc(trainvcmd *rv_vcmd[], int *idx, location l) {
	rv_vcmd[*idx]->type = TRAINVCMD;
	rv_vcmd[*idx]->name = WAITFORLOC;
	rv_vcmd[*idx]->data.waitloc = l;
	(*idx)++;
}

static inline void trainvcmd_addswitch(trainvcmd *rv_vcmd[], int *idx, char const *switchname, char pos) {
	rv_vcmd[*idx]->type = TRAINVCMD;
	rv_vcmd[*idx]->name = SETSWITCH;
	strcpy(rv_vcmd[*idx]->data.switchinfo.nodename, switchname);
	rv_vcmd[*idx]->data.switchinfo.pos = pos;
	(*idx)++;
}

void gps_findpath(gps *this,
		train_descriptor *train,
		location *dest,
		int maxlen,
		trainvcmd *rv_vcmd[],
		int *rv_len) {
	location trainloc;
	train_get_loc(train, &trainloc);

	track_node *src = trainloc.edge->dest;
	track_edge *path[MAX_PATH];
	int pathlen;
	dijkstra(this, src, dest->edge->reverse->dest->reverse, path, &pathlen);
	int cmdlen = 0;


	if (pathlen > 0) {
		fixed dist = gps_distace(&trainloc, dest, path, pathlen);
		int speedidx = train_get_speedidx(train);
		dist = fixed_sub(dist, fixed_sub(fixed_new(trainloc.edge->dist), trainloc.offset));

		if (train_get_speed(train) == 0) {
			trainvcmd_addspeed(rv_vcmd, &cmdlen, CRUISE_SPEED);
			speedidx = train_speed2speedidx(0, CRUISE_SPEED);
		}

		fixed remainingdist = dist;
		fixed stopdist = train_get_stopdist4speedidx(train, speedidx);

		for (int i = 0; i < pathlen; i++) {
			track_edge *nextedge = path[i];
			track_node *nextnode = nextedge->dest;
			fixed nextedgedist = fixed_new(nextedge->dist);
			fixed stopat = fixed_sub(remainingdist, stopdist);
			location stoploc = location_new(nextedge);
			location_add(&stoploc, stopat);

			int needtostop = fixed_sgn(fixed_sub(stopat, nextedgedist)) >= 0;
			int needtoswitch = nextnode->type == NODE_BRANCH
					&& fixed_sgn(fixed_sub(remainingdist, nextedgedist)) > 0;
			int stopfirst = fixed_sgn(fixed_sub(stopat, fixed_new(SWITCH_DIST))) < 0;

			if (needtostop && stopfirst) {
				trainvcmd_addwaitloc(rv_vcmd, &cmdlen, stoploc);
				trainvcmd_addspeed(rv_vcmd, &cmdlen, 0);
			}

			// if this is not the last branch
			if (needtoswitch) {
				location switchloc = location_new(nextedge);
				location_add(&switchloc, fixed_sub(nextedgedist, fixed_new(SWITCH_DIST)));

				track_edge *edgeafterbranch = path[i+1];
				char pos = (edgeafterbranch == &nextnode->edge[DIR_STRAIGHT]) ? 's' : 'c';

				trainvcmd_addwaitloc(rv_vcmd, &cmdlen, switchloc);
				trainvcmd_addswitch(rv_vcmd, &cmdlen, nextnode->name, pos);
			}

			if (needtostop && !stopfirst) {
				trainvcmd_addwaitloc(rv_vcmd, &cmdlen, stoploc);
				trainvcmd_addspeed(rv_vcmd, &cmdlen, 0);
			}

			nextedgedist = fixed_sub(remainingdist, nextedgedist);
			ASSERT(fixed_sgn(remainingdist) >= 0, "remaining dist neg: %F", remainingdist);
		}

		*rv_len = cmdlen;
	} else {
		*rv_len = 0;
	}
}

static inline uint num_neighbour(track_node *n) {
	switch (n->type) {
		case NODE_BRANCH:
			return 2;
		case NODE_EXIT:
			return 0;
		default:
			return 1;
	}
}

// code taken from http://en.wikipedia.org/wiki/Dijkstra's_algorithm#Algorithm
static void dijkstra(gps *this, track_node *src, track_node *tgt, track_edge **rv_edge, int *rv_edgecnt) {
	int const infinity = INT_MAX;
	int dist[TRACK_MAX];
	track_node *previous[TRACK_MAX];
	track_node *nodeary = this->track_node;
	heap *unoptimized = heap_new(TRACK_MAX);

	for (int i = 0; i < TRACK_MAX; i++) {
		previous[i] = NULL;
		if (i != src->num) {
			dist[i] = infinity;
			heap_insert_min(unoptimized, nodeary + i, dist[i]);
		}
	}

	dist[src->num] = 0;
	heap_insert_min(unoptimized, nodeary + src->num, dist[src->num]);

	while(!heap_empty(unoptimized)) {
		track_node *u = heap_extract_min(unoptimized);

		if (u->num == tgt->num) {
			break;
		}
		if (dist[u->num] == infinity) {
			break;
		}

		int neighbour_cnt = num_neighbour(u);

		for (int i = 0; i < neighbour_cnt; i++) {
			track_edge *edge = &u->edge[i];
			track_node *v = edge->dest;
			int alt = dist[u->num] + edge->dist;
			previous[v->num] = u;
			heap_decrease_key(unoptimized, alt, v);
		}
	}

	if (dist[tgt->num] != infinity) {
		track_node *u = tgt;
		int pos = 0;
		do {
			ASSERT(pos < MAX_PATH,  "path size too big!? %d", pos);
			track_node *next = previous[u->num];
			if (next != NULL) {
				if (u->type == NODE_BRANCH && next == u->edge[DIR_CURVED].dest) {
					rv_edge[pos++] = &u->edge[DIR_CURVED];
				} else {
					rv_edge[pos++] = &u->edge[DIR_STRAIGHT];
				}
			}

			u = next;
		} while (u);
		*rv_edgecnt = pos;
	}
	else {
		*rv_edgecnt = 0;
	}
}

static fixed gps_distace(location *start, location *end, track_edge **path, int pathlen) {
	ASSERT((int)path % 4 == 0, "path unaligned");
	ASSERT((int)start % 4 == 0, "start unaligned");
	ASSERT((int)end % 4 == 0, "end unaligned");
	fixed total = fixed_new(0);
	ASSERT(start->edge->dist > 0, "start->edge->dist %d", start->edge->dist);
	total = fixed_add(total, fixed_sub(fixed_new(start->edge->dist), start->offset));
	total = fixed_add(total, end->offset);
	int cnt = 0;

	if (pathlen > 0) {
		track_edge *curedge = path[cnt];
		while(curedge->dest != PREV_NODE(end->edge)) {
			ASSERT(curedge->dist > 0, "curedge->dist %d. curedge:%d",curedge->dist,curedge->num );
			curedge = path[cnt++];
			total = fixed_add(total, fixed_new(curedge->dist));
			PRINT("investigating %s->%s %d %F", PREV_NODE(curedge)->name, curedge->dest->name, curedge->dist, total);
		}
	}

	return total;
}

void gps_test(gps *this) {
	{
		track_node *c14 = &this->track_node[45];
		track_node *mr11 = &this->track_node[101];
		location start = location_new(&c14->edge[0]);
		location end = location_new(&mr11->edge[0]);

		track_edge *path[] = {0};
		fixed result = gps_distace(&start, &end, path, 0);
		ASSERT(fixed_cmp(fixed_new(66), result) == 0, "66 == %F", result);
	}
	{
		track_node *c14 = &this->track_node[45];
		track_node *a15 = &this->track_node[14];

		ASSERT((int)c14 % 4 == 0, "c14 unaligned");
		ASSERT((int)a15 % 4 == 0, "a15 unaligned");
		location start = location_new(&c14->edge[0]);
		location end = location_new(&a15->edge[0]);


		track_edge *path[] = {
				&c14->edge[DIR_STRAIGHT].dest->edge[DIR_STRAIGHT],
				&c14->edge[DIR_STRAIGHT].dest->edge[DIR_STRAIGHT].dest->edge[DIR_CURVED],
				&c14->edge[DIR_STRAIGHT].dest->edge[DIR_STRAIGHT].dest->edge[DIR_CURVED].dest->edge[DIR_STRAIGHT],
		};

		// c14 BR11 	66
		// BR11 BR12 	190
		// BR12 BR4		188
		// BR4 A15		262

		fixed result = gps_distace(&start, &end, path, 3);
		ASSERT(fixed_cmp(result,fixed_new(706)) == 0, "706 == %F", result);
	}
	{
		track_node *c14 = &this->track_node[45];
		track_node *a15 = &this->track_node[14];

		ASSERT((int)c14 % 4 == 0, "c14 unaligned");
		ASSERT((int)a15 % 4 == 0, "a15 unaligned");
		location start = location_new(&c14->edge[0]);
		location_add(&start, fixed_new(20));
		location end = location_new(&a15->edge[0]);
		location_add(&end, fixed_new(40));

		track_edge *path[] = {
				&c14->edge[DIR_STRAIGHT].dest->edge[DIR_STRAIGHT],
				&c14->edge[DIR_STRAIGHT].dest->edge[DIR_STRAIGHT].dest->edge[DIR_CURVED],
				&c14->edge[DIR_STRAIGHT].dest->edge[DIR_STRAIGHT].dest->edge[DIR_CURVED].dest->edge[DIR_STRAIGHT],
		};

		// c14 BR11 	66
		// BR11 BR12 	190
		// BR12 BR4		188
		// BR4 A15		262

		fixed result = gps_distace(&start, &end, path, 3);
		ASSERT(fixed_cmp(result,fixed_new(726)) == 0, "726 == %F", result);
	}
	ExitKernel(0);
}
