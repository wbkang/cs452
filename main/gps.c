#include <gps.h>
#include <syscall.h>
#include <track_data.h>
#include <util.h>
#include <string.h>
#include <engineer.h>
#include <stdio.h>
#include <location.h>
#include <ui/logdisplay.h>

#define CRUISE_SPEED 12
#define REVERSE_TIMEOUT 4000
#define STOP_TIMEOUT 6000

// static fixed gps_distance(location *start, location *end, track_node **path, int pathlen);
static char const *vcmdnames[] = {
	"SETSPEED", "SETREVERSE", "SETSWITCH", "WAITFORMS", "WAITFORLOC", "STOP"
};

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
	rv_vcmd[*idx].type = MSG_TRAINVCMD;
	rv_vcmd[*idx].name = VCMD_SETSPEED;
	rv_vcmd[*idx].data.speed = speed;
	if (speedloc) {
		rv_vcmd[*idx].location = *speedloc;
	} else {
		rv_vcmd[*idx].location = location_undef();
	}
	(*idx)++;
}

static inline void trainvcmd_addstop(trainvcmd *rv_vcmd, int *idx, location *stoploc) {
	rv_vcmd[*idx].type = MSG_TRAINVCMD;
	rv_vcmd[*idx].name = VCMD_STOP;
	rv_vcmd[*idx].location = *stoploc;
	(*idx)++;
}

static inline void trainvcmd_addreverse(trainvcmd *rv_vcmd, int *idx, location *revloc) {
	rv_vcmd[*idx].type = MSG_TRAINVCMD;
	rv_vcmd[*idx].name = VCMD_SETREVERSE;
	rv_vcmd[*idx].location = *revloc;
	(*idx)++;
}

static inline void trainvcmd_addpause(trainvcmd *rv_vcmd, int *idx, int ms) {
	rv_vcmd[*idx].type = MSG_TRAINVCMD;
	rv_vcmd[*idx].name = VCMD_WAITFORMS;
	rv_vcmd[*idx].data.timeout = ms;
	(*idx)++;
}

static inline void trainvcmd_addswitch(trainvcmd *rv_vcmd, int *idx, char const *switchname, char pos, location *switchloc) {
	rv_vcmd[*idx].type = MSG_TRAINVCMD;
	rv_vcmd[*idx].name = VCMD_SETSWITCH;
	strcpy(rv_vcmd[*idx].data.switchinfo.nodename, switchname);
	rv_vcmd[*idx].data.switchinfo.pos = pos;
	rv_vcmd[*idx].location = *switchloc;
	(*idx)++;
}

static inline char gps_getnextswitchpos(track_node *sw, track_edge *edgeafterbranch) {
	if (edgeafterbranch == &sw->edge[DIR_STRAIGHT]) return 's';
	if (edgeafterbranch == &sw->edge[DIR_CURVED]) return 'c';
	ASSERT(0, "edgeafterbranch not an edge of sw");
	return '\0'; // unreachable
}

static inline void gps_get_rev_stoploc(train *train, track_node *node, location *rv_loc) {
	ASSERTNOTNULL(node);
	*rv_loc = location_fromnode(node, 0);
	if (node->type == NODE_MERGE) {
		ASSERT(0, "invalid path, yo");
		int len = train_get_length(train) + train_get_poserr(train);
		location_add(rv_loc, fixed_new(len));
	}
}

static int gps_collapsereverse(track_node *startnode, track_node **path, int pathlen, train *train) {
	if (pathlen == 0 || train_is_moving(train)) return -1;
	for (int i = 0; i < pathlen; i++) {
		if (path[i] == startnode->reverse) return i;
	}
	return -1;
}

void gps_findpath(gps *this, train *train, location *dest, int maxlen, trainvcmd *rv_vcmd, int *rv_len, logdisplay *log) {
	location trainloc = train_get_frontloc(train);

	track_node *src = trainloc.edge->dest;
	track_node **path = train->path->nodes;
	int *pathlen = &train->path->pathlen;

	dijkstra(this->track_node, this->heap_dijkstra, src, dest->edge->src, path, pathlen, train);

	// char buf[100];
	// location_tostring(&trainloc, buf);
	// logdisplay_printf(log, "dj: %s", buf);
	// for (int i = 0; i < *pathlen; i++) {
	// 	logdisplay_printf(log, "->%s", path[i]->name);
	// }
	// logdisplay_flushline(log);

	int cmdlen = 0;

	if (*pathlen <= 0) {
		// trainvcmd_addspeed(rv_vcmd, &cmdlen, 0, NULL);
		// trainvcmd_addpause(rv_vcmd, &cmdlen, random() & (NO_PATH_RANDOM_WAIT - 1));
		// *pathlen = cmdlen;
		*rv_len = 0;
		return;
	}

	int collapseidx = gps_collapsereverse(trainloc.edge->src, path, *pathlen, train);
	int startidx;

	if (collapseidx >= 0) {
		trainvcmd_addreverse(rv_vcmd, &cmdlen, NULL);
		trainvcmd_addspeed(rv_vcmd, &cmdlen, CRUISE_SPEED, NULL);
		startidx = collapseidx;
	} else {
		trainvcmd_addspeed(rv_vcmd, &cmdlen, CRUISE_SPEED, &trainloc);
		startidx = 0;
	}

	for (int i = startidx; i < *pathlen - 1; i++) {
		track_node *curnode = path[i];
		track_node *nextnode = path[i + 1];
		track_edge *nextedge = track_get_edge(curnode, nextnode);
		ASSERT(nextedge || curnode->reverse == nextnode, "i: %d curnode: %s, nextnode: %s", i, curnode->name, nextnode->name);

		if (nextedge) {
			if (curnode->type == NODE_BRANCH) {
				char pos = gps_getnextswitchpos(curnode, nextedge);
				location switchloc = location_fromedge(nextedge);
				trainvcmd_addswitch(rv_vcmd, &cmdlen, curnode->name, pos, &switchloc);
			} else {
				// nothing to do
			}
		} else if (curnode->reverse == nextnode) { // reverse plan
			location stoploc;
			gps_get_rev_stoploc(train, curnode, &stoploc);

			trainvcmd_addstop(rv_vcmd, &cmdlen, &stoploc);
			trainvcmd_addpause(rv_vcmd, &cmdlen, REVERSE_TIMEOUT);
			trainvcmd_addreverse(rv_vcmd, &cmdlen, &stoploc);
			// @TODO: this works but the stop at the end is useless
			if (i < *pathlen - 2) {
				trainvcmd_addspeed(rv_vcmd, &cmdlen, CRUISE_SPEED, NULL);
			}
		} else {
			ASSERT(0, "no edge between %s and %s", curnode->name, nextnode->name);
		}
	}

	track_node *last_node = path[*pathlen - 1];

	if (last_node->type == NODE_BRANCH) {
		char pos = gps_getnextswitchpos(last_node, dest->edge);
		location switchloc = location_fromedge(dest->edge);
		trainvcmd_addswitch(rv_vcmd, &cmdlen, last_node->name, pos, &switchloc);
	}

	// if (last_node == trainloc.edge->dest) {
	// 	track_node *cur_node = trainloc.edge->dest;
	// 	track_node *next_node = trainloc.edge->dest->reverse;
	// 	location stoploc;
	// 	gps_get_rev_stoploc(train, cur_node, &stoploc);
	// 	trainvcmd_addstop(rv_vcmd, &cmdlen, &stoploc);
	// 	trainvcmd_addpause(rv_vcmd, &cmdlen, REVERSE_TIMEOUT);
	// 	trainvcmd_addreverse(rv_vcmd, &cmdlen, &stoploc);
	// 	trainvcmd_addspeed(rv_vcmd, &cmdlen, CRUISE_SPEED, NULL);
	// }

//			ExitKernel(0);
	trainvcmd_addstop(rv_vcmd, &cmdlen, dest);
	trainvcmd_addpause(rv_vcmd, &cmdlen, STOP_TIMEOUT);

	*rv_len = cmdlen;
}

static int dist_between(track_node *u, track_edge *e, track_node *v, train *train) {
	int train_no = train->no;
	if (v->type == NODE_MERGE) {
		track_node *v_rev = v->reverse;
		if (!can_occupy(&v_rev->edge[0], train_no)) return infinity;
		if (!can_occupy(&v_rev->edge[1], train_no)) return infinity;
	} else if (u->type == NODE_BRANCH) {
		if (!can_occupy(&u->edge[0], train_no)) return infinity;
		if (!can_occupy(&u->edge[1], train_no)) return infinity;
	} else {
		if (!can_occupy(e, train_no)) return infinity;
	}
	return e->dist;
}

// code taken from http://en.wikipedia.org/wiki/Dijkstra's_algorithm#Algorithm
// @TODO: keep this algorithm independent of gps, pass in whats needed as args
void dijkstra(track_node *nodes, heap *Q, track_node *src, track_node *dest, track_node *rv_nodes[], int *rv_len_nodes, train *train) {
	int dist[TRACK_MAX];
	track_node *previous[TRACK_MAX];
	heap_clear(Q);

	for (int i = 0; i < TRACK_MAX; i++) {
		track_node *node = &nodes[i];
		int cost = (node == src) ? 0 : infinity;
		dist[i] = cost;
		heap_min_insert(Q, node, cost);

		previous[i] = NULL;
	}

	while (!heap_empty(Q)) {
		track_node *u = heap_min_extract(Q);
		int uidx = u - nodes;

		if (dist[uidx] == infinity) break;

		if (u == dest) break; // the path to u==dest is optimized, stop looking

		{
			for (int i = track_numedges(u) - 1; i >= 0; --i) {
				track_edge *edge = &u->edge[i];
				track_node *v = edge->dest;
				int dist_btwn = dist_between(u, edge, v, train);
				if (dist_btwn == infinity) continue;
				int alt = dist[uidx] + dist_btwn;
				int vidx = v - nodes;
				if (alt < dist[vidx]) {
					dist[vidx] = alt;
					previous[vidx] = u;
					heap_min_decrease_key(Q, v, alt);
				}
			}
		}
		{
			track_node *u_rev = u->reverse;
			// @TODO: for now we reverse only on landmarks, so this is not possible
			if (u_rev == dest) continue;
			int u_rev_idx = u_rev - nodes;
			int rev_dist = train_get_reverse_cost(train, dist[uidx], u);
			if (rev_dist == infinity) continue;
			int alt = dist[uidx] + rev_dist;
			if (alt < dist[u_rev_idx]) {
				dist[u_rev_idx] = alt;
				previous[u_rev_idx] = u;
				heap_min_decrease_key(Q, u_rev, alt);
			}
		}
	}

	track_node *u = dist[dest - nodes] < infinity ? dest : NULL;

	// if (!u) {
	// 	int maxdist = 0;
	// 	for (int i = 0; i < TRACK_MAX; i++) {
	// 		if (dist[i] < infinity && dist[i] > maxdist) {
	// 			maxdist = dist[i];
	// 			u = &nodes[i];
	// 		}
	// 	}
	// }

	if (u) {
		int uidx = u - nodes;
		int len = 0;

		while (u) {
			ASSERT(len < MAX_PATH, "path size too big!? %d", len);
			rv_nodes[len++] = u;
			u = previous[uidx];
			uidx = u - nodes;
		}

		REVERSE_ARR(rv_nodes, len);

		*rv_len_nodes = len;
	} else {
		*rv_len_nodes = 0;
	}
}

// static fixed gps_distance(location *start, location *end, track_node **path, int pathlen) {
// 	ASSERT((int) path & 3 == 0, "path unaligned");
// 	ASSERT((int) start & 3 == 0, "start unaligned");
// 	ASSERT((int) end & 3 == 0, "end unaligned");
// 	fixed total = fixed_new(0);
// 	ASSERT(start->edge->dist > 0, "start->edge->dist %d", start->edge->dist);
// 	total = fixed_add(total, fixed_sub(fixed_new(start->edge->dist), start->offset));
// 	total = fixed_add(total, end->offset);
// 	int cnt = 0;
// 	char startbuf[100], endbuf[100];
// 	location_tostring(start, startbuf);
// 	location_tostring(end, endbuf);

// 	if (pathlen > 0) {
// 		track_node *curnode = path[cnt];
// 		while (curnode != end->edge->src) {
// 			track_edge * const curedge = track_get_edge(curnode, path[cnt+1]);
// 			ASSERT(curedge || curnode->reverse == path[cnt+1], "wtf");
// 			if (curedge) { // this is a non reverse segment
// 				total = fixed_add(total, fixed_new(curedge->dist));
// 			}
// 			curnode = path[++cnt];
// 		}
// 	}

// 	return total;
// }

int vcmd2str(char *buf, trainvcmd *vcmd) {
	char * const origbuf = buf;
	buf += sprintf(buf, "[%s: ", vcmdnames[vcmd->name]);
	switch (vcmd->name) {
		case VCMD_WAITFORLOC:
			buf += sprintf("wait for ", buf);
			buf += location_tostring(&vcmd->location, buf);
			break;
		case VCMD_STOP:
			buf += sprintf("stop at ", buf);
			buf += location_tostring(&vcmd->location, buf);
			break;
		case VCMD_SETSPEED:
			buf += sprintf(buf, "speed %d at ", vcmd->data.speed);
			buf += location_tostring(&vcmd->location, buf);
			break;
		case VCMD_SETSWITCH:
			buf += sprintf(buf, "switch %s to %c at ", vcmd->data.switchinfo.nodename, vcmd->data.switchinfo.pos);
			buf += location_tostring(&vcmd->location, buf);
			break;
		case VCMD_SETREVERSE:
			buf += sprintf(buf, "reverse");
			break;
		case VCMD_WAITFORMS:
			buf += sprintf(buf, "pause for %dms", vcmd->data.timeout);
			break;
	}
	buf += sprintf(buf, "]");
	return buf - origbuf;
}

//void gps_test(gps *this, engineer *eng) {
//	{
//		track_node *c14 = &this->track_node[45];
//		track_node *mr11 = &this->track_node[101];
//		location start = location_fromnode(c14, 0);
//		location end = location_fromnode(mr11, 0);
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
//		location start = location_fromnode(c14, 0);
//		location end = location_fromnode(a15, 0);
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
//		location start = location_fromnode(c14, 0);
//		location_add(&start, fixed_new(20));
//		location end = location_fromnode(a15, 0);
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
//		train *train = &eng->train[38];
//
//		track_node *c14 = &eng->track_nodes_arr[45];
//		track_node *c10 = &eng->track_nodes_arr[41];
//		location dest;
//		dest = location_fromedge(c10, 0);
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
