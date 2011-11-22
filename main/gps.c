#include <gps.h>
#include <syscall.h>
#include <track_data.h>
#include <util.h>
#include <string.h>
#include <engineer.h>
#include <stdio.h>
#include <location.h>

#define MAX_PATH 160
#define CRUISE_SPEED 10

static void dijkstra(track_node *nodeary, heap *unoptimized, track_node *src, track_node *tgt, track_edge **rv_edge, int *rv_edgecnt, int stopdist);
static fixed gps_distace(location *start, location *end, track_edge **path, int pathlen);
static char const *vcmdnames[] = { "SETSPEED", "SETREVERSE", "SETSWITCH", "WAITFORMS", "WAITFORLOC", "STOP" };

gps *gps_new(track_node *nodes) {
	gps *this = malloc(sizeof(gps));
	ASSERT(sizeof(gps) > 0, "size of gps is zero");
	ASSERT(this, "out of memory");
	ASSERT(nodes, "size of gps is zero");
	this->track_node = nodes;
	this->heap_dijkstra = heap_new(TRACK_MAX);
	return this;
}

static inline void trainvcmd_addspeed(trainvcmd *rv_vcmd, int *idx, int speed, location *speedloc) {
	rv_vcmd[*idx].type = TRAINVCMD;
	rv_vcmd[*idx].name = VCMD_SETSPEED;
	rv_vcmd[*idx].data.speed = speed;
	rv_vcmd[*idx].location = *speedloc;
	(*idx)++;
}

static inline void trainvcmd_addstop(trainvcmd *rv_vcmd, int *idx, location *stoploc) {
	rv_vcmd[*idx].type = TRAINVCMD;
	rv_vcmd[*idx].name = VCMD_STOP;
	rv_vcmd[*idx].location = *stoploc;
	(*idx)++;
}

static inline void trainvcmd_addreverse(trainvcmd *rv_vcmd, int *idx, location *revloc) {
	rv_vcmd[*idx].type = TRAINVCMD;
	rv_vcmd[*idx].name = VCMD_SETREVERSE;
	rv_vcmd[*idx].location = *revloc;
	(*idx)++;
}

static inline void trainvcmd_addswitch(trainvcmd *rv_vcmd, int *idx, char const *switchname, char pos, location *switchloc) {
	rv_vcmd[*idx].type = TRAINVCMD;
	rv_vcmd[*idx].name = VCMD_SETSWITCH;
	strcpy(rv_vcmd[*idx].data.switchinfo.nodename, switchname);
	rv_vcmd[*idx].data.switchinfo.pos = pos;
	rv_vcmd[*idx].location = *switchloc;
	(*idx)++;
}

static inline char gps_getnextswitchpos(track_node *sw, track_edge *edgeafterbranch) {
	return (edgeafterbranch == &sw->edge[DIR_STRAIGHT]) ? 's' : 'c';
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
	// TODO now using arbitrary reverse distance
	dijkstra(this->track_node, this->heap_dijkstra, src, dest->edge->src, path, &pathlen, 0);

	int cmdlen = 0;

	if (pathlen > 0) {
		fixed dist = gps_distace(&trainloc, dest, path, pathlen);
		dist = fixed_sub(dist, fixed_sub(fixed_new(trainloc.edge->dist), trainloc.offset));

		if (train_get_speed(train) == 0) {
			trainvcmd_addspeed(rv_vcmd, &cmdlen, CRUISE_SPEED, &trainloc);
		}

		for (int i = 0; i < pathlen; i++) {
			track_edge *nextedge = path[i];
			track_node *curnode = path[i]->src;
			if (curnode->type == NODE_BRANCH) {
				char pos = gps_getnextswitchpos(curnode, nextedge);
				location switchloc = location_new(nextedge);

				trainvcmd_addswitch(rv_vcmd, &cmdlen, curnode->name, pos, &switchloc);
			}

			// reverse plan
			if (i + 1 < pathlen) {
				track_node *curnode_rev = curnode->reverse;
				track_edge *nextnextedge = path[i + 1];
				if (nextnextedge->src == curnode_rev) {
					trainvcmd_addstop(rv_vcmd, &cmdlen, curnode);
					trainvcmd_addreverse(rv_vcmd, &cmdlen, curnode);
					trainvcmd_addspeed(rv_vcmd, &cmdlen, CRUISE_SPEED, nextnextedge->src);
				}
			}
		}

		trainvcmd_addstop(rv_vcmd, &cmdlen, dest);

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
static void dijkstra(track_node *nodeary, heap *unoptimized, track_node *src, track_node *tgt, track_edge **rv_edge, int *rv_edgecnt, int stopdist) {
	int const reverse_dist = stopdist ? stopdist * 2 : 1000; // TODO fix this
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
		track_node *u_rev = u->reverse;
		int rev_neighbour_cnt = num_neighbour(u_rev);
		for (int i = 0; i < neighbour_cnt; i++) {
			track_edge *edge = &u_rev->edge[i];
			track_node *v = edge->dest;
			int vidx = v - nodeary;

			int alt = (dist[vidx] == infinity) ? edge->dist : dist[vidx] + edge->dist;
			alt += reverse_dist;

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
		while (curedge->dest != PREV_NODE(end->edge)) {
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
	char locname[100];
	buf += sprintf(buf, "[%s ", vcmdnames[vcmd->name]);
	switch (vcmd->name) {
//		case VCMD_WAITFORLOC:
//			buf += sprintf(buf, "src:%s offset:%F", vcmd->data.waitloc.edge->src->name, vcmd->data.waitloc.offset);
//			break;
		case VCMD_STOP:
			location_tostring(&vcmd->location, locname);
			buf += sprintf(buf, "stop at %s", locname);
			break;
		case VCMD_SETSPEED:
			location_tostring(&vcmd->location, locname);
			buf += sprintf(buf, "speed:%d at %s", vcmd->data.speed, locname);
			break;
		case VCMD_SETSWITCH:
			location_tostring(&vcmd->location, locname);
			buf += sprintf(buf, "switch %s to %c at %s", vcmd->data.switchinfo.nodename, vcmd->data.switchinfo.pos, locname);
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
