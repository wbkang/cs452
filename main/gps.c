#include <gps.h>
#include <syscall.h>
#include <track_data.h>
#include <util.h>
#include <string.h>
#include <engineer.h>
#include <stdio.h>

#define MAX_PATH 160
#define SWITCH_DIST 50
#define CRUISE_SPEED 12

static void dijkstra(track_node *nodeary, heap *unoptimized, track_node *src, track_node *tgt, track_edge **rv_edge, int *rv_edgecnt);
static fixed gps_distace(location *start, location *end, track_edge **path, int pathlen);
static char const *vcmdnames[] = { "SETSPEED", "SETREVERSE", "SETSWITCH", "WAITFORMS", "WAITFORLOC" };

gps *gps_new(track_node *nodes) {
	gps *this = malloc(sizeof(gps));
	ASSERT(sizeof(gps) > 0, "size of gps is zero");
	ASSERT(this, "out of memory");
	ASSERT(nodes, "size of gps is zero");
	this->track_node = nodes;
	this->heap_dijkstra = heap_new(TRACK_MAX);
	return this;
}

static inline void trainvcmd_addspeed(trainvcmd *rv_vcmd, int *idx, int speed) {
	rv_vcmd[*idx].type = TRAINVCMD;
	rv_vcmd[*idx].name = VCMD_SETSPEED;
	rv_vcmd[*idx].data.speed = speed;
	(*idx)++;
}

static inline void trainvcmd_addwaitloc(trainvcmd *rv_vcmd, int *idx, location l) {
	rv_vcmd[*idx].type = TRAINVCMD;
	rv_vcmd[*idx].name = VCMD_WAITFORLOC;
	rv_vcmd[*idx].data.waitloc = l;
	(*idx)++;
}

static inline void trainvcmd_addswitch(trainvcmd *rv_vcmd, int *idx, char const *switchname, char pos) {
	rv_vcmd[*idx].type = TRAINVCMD;
	rv_vcmd[*idx].name = VCMD_SETSWITCH;
	strcpy(rv_vcmd[*idx].data.switchinfo.nodename, switchname);
	rv_vcmd[*idx].data.switchinfo.pos = pos;
	(*idx)++;
}

static inline char gps_getnextswitchpos(track_node *sw, track_edge *edgeafterbranch) {
	return (edgeafterbranch == &sw->edge[DIR_STRAIGHT]) ? 's' : 'c';
}

static inline void gps_get_closest_switch(track_edge const *nextedge,
		track_node **rv_closest_switch_node,
		fixed *rv_closest_switch_dist) {
	int const max_search_depth = 10;


	fixed switchdist = fixed_new(nextedge->dist);
	track_node *cur_node = nextedge->dest;
	for (int i = 0; i < max_search_depth; i++) {
		if (cur_node->type == NODE_BRANCH) {
			*rv_closest_switch_node = cur_node;
			*rv_closest_switch_dist = switchdist;
			return;
		} else if (cur_node->type == NODE_EXIT) {
			break;
		} else {
			cur_node = cur_node->edge[0].dest;
		}
	}
	*rv_closest_switch_node = NULL;
}

void gps_findpath(gps *this,
		train_descriptor *train,
		location *dest,
		int maxlen,
		trainvcmd *rv_vcmd,
		int *rv_len) {
	location trainloc;
	train_get_loc(train, &trainloc);

	track_node *src = trainloc.edge->dest;
	track_edge *path[MAX_PATH];
	int pathlen;
	dijkstra(this->track_node, this->heap_dijkstra, src, dest->edge->src, path, &pathlen);

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

		if ((pathlen > 0 || trainloc.edge != dest->edge) && trainloc.edge->dest->type == NODE_BRANCH) {
			trainvcmd_addswitch(rv_vcmd, &cmdlen, trainloc.edge->dest->name, gps_getnextswitchpos(trainloc.edge->dest, path[0]));
		}
		for (int i = 0; i < pathlen; i++) {
			track_edge *nextedge = path[i];
			track_node *nextnode = nextedge->dest;
			fixed nextedgedist = fixed_new(nextedge->dist);
			fixed stopat = fixed_sub(remainingdist, stopdist);
			location stoploc = location_new(nextedge);
			location_add(&stoploc, stopat);
			track_node *closest_switch_node;
			fixed closest_switch_dist = fixed_new(0);
			gps_get_closest_switch(nextedge, &closest_switch_node, &closest_switch_dist);

			int needtostop = fixed_cmp(stopat, nextedgedist) < 0 && fixed_sgn(stopat) >= 0;
			int needtoswitch = closest_switch_node // exists
					&& closest_switch_node->type == NODE_BRANCH // branch
					&& fixed_cmp(remainingdist, closest_switch_dist) > 0 // do we need to go past the switch
					&& fixed_cmp(fixed_sub(closest_switch_dist, fixed_new(SWITCH_DIST)), nextedgedist); // do we need to switch in the current edge
			int stopfirst = fixed_sgn(fixed_sub(stopat, fixed_new(SWITCH_DIST))) < 0;

			if (needtostop && stopfirst) {
				trainvcmd_addwaitloc(rv_vcmd, &cmdlen, stoploc);
				trainvcmd_addspeed(rv_vcmd, &cmdlen, 0);
			}

			// if this is not the last branch
			if (needtoswitch) {
				location switchloc = location_new(nextedge);
				fixed switch_at = fixed_sub(closest_switch_dist, fixed_new(SWITCH_DIST));
				location_add(&switchloc, switch_at);
				track_edge *edgeafterbranch = path[i+1];

				trainvcmd_addwaitloc(rv_vcmd, &cmdlen, switchloc);
				trainvcmd_addswitch(rv_vcmd, &cmdlen, nextnode->name, gps_getnextswitchpos(nextnode, edgeafterbranch));
			}

			if (needtostop && !stopfirst) {
				trainvcmd_addwaitloc(rv_vcmd, &cmdlen, stoploc);
				trainvcmd_addspeed(rv_vcmd, &cmdlen, 0);
			}

			remainingdist = fixed_sub(remainingdist, nextedgedist);
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
// @TODO: keep this algorithm independent of gps, pass in whats needed as args
static void dijkstra(track_node *nodeary, heap *unoptimized, track_node *src, track_node *tgt, track_edge **rv_edge, int *rv_edgecnt) {
	int const infinity = INT_MAX;
	int dist[TRACK_MAX];
	track_node *previous[TRACK_MAX];
	heap_clear(unoptimized);
	int srcidx = src - nodeary;
	int tgtidx = tgt - nodeary;

	for (int i = 0; i < TRACK_MAX; i++) {
		track_node *node = &nodeary[i];
		previous[i] = NULL;
		if (node != src) {
			dist[i] = infinity;
			heap_insert_min(unoptimized, node, dist[i]);
		}
	}

	dist[srcidx] = 0;
	heap_insert_min(unoptimized, src, dist[srcidx]);

	while(!heap_empty(unoptimized)) {
		track_node *u = heap_extract_min(unoptimized);
		int uidx = u - nodeary;

		if (u == tgt) {
			break;
		}

		if (dist[uidx] == infinity) {
			break;
		}

		int neighbour_cnt = num_neighbour(u);

		for (int i = 0; i < neighbour_cnt; i++) {
			track_edge *edge = &u->edge[i];
			track_node *v = edge->dest;
			int vidx = v - nodeary;

			int alt = (dist[vidx] == infinity) ? edge->dist : dist[vidx] + edge->dist;

			if (alt < dist[vidx]) {
				dist[vidx] = alt;
				previous[vidx] = u;
				heap_decrease_key_min(unoptimized, v, alt);
			}
		}
	}

	if (dist[tgtidx] != infinity) {
		track_node *u = tgt;
		int uidx = u - nodeary;
		int pos = 0;

		while (u) {
			ASSERT(pos < MAX_PATH,  "path size too big!? %d", pos);
			track_node *next = previous[uidx];
			if (next != NULL) {
				track_node *ureversed = u->reverse;
				if (ureversed->type == NODE_BRANCH && ureversed->edge[DIR_CURVED].dest->reverse == next) {
					rv_edge[pos++] = ureversed->edge[DIR_CURVED].reverse;
				} else {
					rv_edge[pos++] = ureversed->edge[DIR_STRAIGHT].reverse;
				}
			}

			u = next;
			uidx = u - nodeary;
		}

		for (int i = 0; i < pos / 2; i++) { // reverse the path
			track_edge *e = rv_edge[i];
			rv_edge[i] = rv_edge[pos - i - 1];
			rv_edge[pos - i - 1] = e;
		}


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
//			PRINT("investigating %s->%s %d %F", PREV_NODE(curedge)->name, curedge->dest->name, curedge->dist, total);
		}
	}

	return total;
}

int vcmd2str(char *buf, trainvcmd *vcmd) {
	char * const origbuf = buf;
	buf += sprintf(buf, "[%s ", vcmdnames[vcmd->name]);
	switch(vcmd->name) {
		case VCMD_WAITFORLOC:
			buf += sprintf(buf, "src:%s offset:%F", vcmd->data.waitloc.edge->src->name, vcmd->data.waitloc.offset);
			break;
		case VCMD_SETSPEED:
			buf += sprintf(buf, "speed:%d", vcmd->data.speed);
			break;
		case VCMD_SETSWITCH:
			buf += sprintf(buf, "switch %s to %c", vcmd->data.switchinfo.nodename, vcmd->data.switchinfo.pos);
			break;
		case VCMD_SETREVERSE:
			buf += sprintf(buf, "reverse");
			break;
		case VCMD_WAITFORMS:
			buf += sprintf(buf, "for %dms", vcmd->data.timeout);
			break;
	}
	buf += sprintf(buf, "]");
	return buf - origbuf;
}

//void gps_test(gps *this, engineer *eng) {
//	{
//		track_node *c14 = &this->track_node[45];
//		track_node *mr11 = &this->track_node[101];
//		location start = location_new(&c14->edge[0]);
//		location end = location_new(&mr11->edge[0]);
//
//		track_edge *path[] = {0};
//		fixed result = gps_distace(&start, &end, path, 0);
//		ASSERT(fixed_cmp(fixed_new(66), result) == 0, "66 == %F", result);
//	}
//	{
//		track_node *c14 = &this->track_node[45];
//		track_node *a15 = &this->track_node[14];
//
//		ASSERT((int)c14 % 4 == 0, "c14 unaligned");
//		ASSERT((int)a15 % 4 == 0, "a15 unaligned");
//		location start = location_new(&c14->edge[0]);
//		location end = location_new(&a15->edge[0]);
//
//
//		track_edge *path[] = {
//				&c14->edge[DIR_STRAIGHT].dest->edge[DIR_STRAIGHT],
//				&c14->edge[DIR_STRAIGHT].dest->edge[DIR_STRAIGHT].dest->edge[DIR_CURVED],
//				&c14->edge[DIR_STRAIGHT].dest->edge[DIR_STRAIGHT].dest->edge[DIR_CURVED].dest->edge[DIR_STRAIGHT],
//		};
//
//		// c14 BR11 	66
//		// BR11 BR12 	190
//		// BR12 BR4		188
//		// BR4 A15		262
//
//		fixed result = gps_distace(&start, &end, path, 3);
//		ASSERT(fixed_cmp(result,fixed_new(706)) == 0, "706 == %F", result);
//	}
//	{
//		track_node *c14 = &this->track_node[45];
//		track_node *a15 = &this->track_node[14];
//
//		ASSERT((int)c14 % 4 == 0, "c14 unaligned");
//		ASSERT((int)a15 % 4 == 0, "a15 unaligned");
//		location start = location_new(&c14->edge[0]);
//		location_add(&start, fixed_new(20));
//		location end = location_new(&a15->edge[0]);
//		location_add(&end, fixed_new(40));
//
//		track_edge *path[] = {
//				&c14->edge[DIR_STRAIGHT].dest->edge[DIR_STRAIGHT],
//				&c14->edge[DIR_STRAIGHT].dest->edge[DIR_STRAIGHT].dest->edge[DIR_CURVED],
//				&c14->edge[DIR_STRAIGHT].dest->edge[DIR_STRAIGHT].dest->edge[DIR_CURVED].dest->edge[DIR_STRAIGHT],
//		};
//
//		// c14 BR11 	66
//		// BR11 BR12 	190
//		// BR12 BR4		188
//		// BR4 A15		262
//
//		fixed result = gps_distace(&start, &end, path, 3);
//		ASSERT(fixed_cmp(result,fixed_new(726)) == 0, "726 == %F", result);
//	}
//	{
//		track_node *c14 = &this->track_node[45];
//		track_node *a15 = &this->track_node[14];
//		track_edge *rv[100];
//		int cnt;
//		dijkstra(this->track_node, this->heap_dijkstra, c14, a15, rv, &cnt);
//		ASSERT(cnt == 4, "cnt != 4. %d", cnt);
//	}
//	{
//		track_node *c14 = &this->track_node[45];
//		track_node *c10 = &this->track_node[41];
//		track_edge *rv[100];
//		int cnt;
//		dijkstra(this->track_node, this->heap_dijkstra, c14, c10, rv, &cnt);
//
////			PRINT("LEN: %d", cnt);
////			for ( int i = 0 ; i < cnt; i++) {
////				PRINT("PATH: %s->%s", PREV_NODE(rv[i])->name, rv[i]->dest->name);
////			}
//		ASSERT(cnt == 6, "cnt != 6! %d", cnt);
//	}
//	{
//		train_descriptor *train = &eng->train[38];
//
//		track_node *c14 = &eng->track_nodes_arr[45];
//		track_node *c10 = &eng->track_nodes_arr[41];
//		location dest;
//		dest = location_new(&c10->edge[0]);
//
//
//		trainvcmd vcmds[100];
//		int rv_len;
//		gps_findpath(this, train, &dest, 100, vcmds, &rv_len);
//
////		PRINT("rv_len:%d", rv_len);
////
////		for (int i = 0; i < rv_len; i++) {
////			trainvcmd *vcmd = &vcmds[i];
////			char buf[1000];
////			vcmd2str(buf, vcmd);
//////			PRINT("%d:%s", i, buf);
//////			logdisplay_flushline(eng->log);
////		}
//	}
////	ExitKernel(0);
//}
