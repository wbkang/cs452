#include <gps.h>
#include <syscall.h>
#include <track_data.h>
#include <util.h>
#include <string.h>
#include <engineer.h>
#include <stdio.h>
#include <location.h>
#include <ui/logdisplay.h>

#define MAX_PATH 160
#define CRUISE_SPEED 10
#define REVERSE_TIMEOUT 4000
#define REVERSE_COST 500

static fixed gps_distance(location *start, location *end, track_node **path, int pathlen);
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
	if (speedloc) {
		rv_vcmd[*idx].location = *speedloc;
	} else {
		rv_vcmd[*idx].location = location_undef();
	}
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

static inline void trainvcmd_addpause(trainvcmd *rv_vcmd, int *idx, int ms) {
	rv_vcmd[*idx].type = TRAINVCMD;
	rv_vcmd[*idx].name = VCMD_WAITFORMS;
	rv_vcmd[*idx].data.timeout = ms;
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


static inline track_edge *gps_gettrackedge(track_node *from, track_node *to) {
	if (from->type == NODE_BRANCH && from->edge[1].dest == to) {
		return &from->edge[1];
	} else if (from->edge[0].dest == to) {
		return &from->edge[0];
	} else {
//		ASSERT(0, "no direct edge from %s to %s", from->name, to->name);
		return 0; // unreachable
	}
}

static inline void gps_getstoploc(location *loc, track_node *cur_node, track_node *next_node, train_descriptor *train) {
	ASSERTNOTNULL(loc);
	ASSERTNOTNULL(cur_node);
	ASSERTNOTNULL(next_node);
	if (cur_node->type == NODE_EXIT) {
		ASSERTNOTNULL(&next_node->edge[0]);
		*loc = location_new(&next_node->edge[0]);
		location_add(loc, fixed_new(50));
		location_reverse(loc);
	} else {
		ASSERTNOTNULL(&cur_node->edge[0]);
		*loc = location_new(&cur_node->edge[0]);
		location_add(loc, fixed_new(train_get_train_length(train) << 1));
	}
}

void gps_findpath(gps *this,
		train_descriptor *train,
		location *dest,
		int maxlen,
		trainvcmd *rv_vcmd,
		int *rv_len,
		logdisplay *log) {
	location trainloc;
	train_get_loc(train, &trainloc);

	track_node *src = trainloc.edge->dest;
	track_node *path[MAX_PATH];
	int pathlen;
	// TODO now using arbitrary reverse distance
	dijkstra(this->track_node, this->heap_dijkstra, src, dest->edge->src, path, &pathlen, train);

	char buf[100];
	location_tostring(&trainloc, buf);
	logdisplay_printf(log, "dj:%s", buf);
	for ( int i = 0 ; i < pathlen; i++) {
		logdisplay_printf(log, "->%s", path[i]->name);
	}
	logdisplay_flushline(log);
//
//	char buf[100];
//	location_tostring(&trainloc, buf);
//	PRINT( "dj:%s", buf);
//	for ( int i = 0 ; i < pathlen; i++) {
//		PRINT("%d->%s", i, path[i]->name);
//	}
//	ExitKernel(0);

	int cmdlen = 0;

	if (pathlen > 0) {
		fixed dist = gps_distance(&trainloc, dest, path, pathlen);
		dist = fixed_sub(dist, fixed_sub(fixed_new(trainloc.edge->dist), trainloc.offset));

		// reverse case
		if (path[0] == trainloc.edge->dest->reverse) {
			if (fixed_sgn(train_get_velocity(train))<= 0 && pathlen >= 3) {
				trainvcmd_addreverse(rv_vcmd, &cmdlen, NULL);
				trainvcmd_addspeed(rv_vcmd, &cmdlen, CRUISE_SPEED, NULL);
			} else {
				track_node *cur_node = trainloc.edge->dest;
				location stoploc;
				gps_getstoploc(&stoploc, cur_node, path[0], train);

				trainvcmd_addstop(rv_vcmd, &cmdlen, &stoploc);
				trainvcmd_addpause(rv_vcmd, &cmdlen, REVERSE_TIMEOUT);
				trainvcmd_addreverse(rv_vcmd, &cmdlen, &stoploc);
				trainvcmd_addspeed(rv_vcmd, &cmdlen, CRUISE_SPEED, NULL);
			}
		} else {
			trainvcmd_addspeed(rv_vcmd, &cmdlen, CRUISE_SPEED, &trainloc);
		}

		for (int i = 0; i < pathlen - 1; i++) {
			track_node *curnode = path[i];
			track_node *nextnode = path[i + 1];
			track_edge *nextedge = gps_gettrackedge(curnode, nextnode);
			ASSERT(nextedge || curnode->reverse == nextnode, "i:%d curnode:%s, nextnode:%s", i, curnode->name, nextnode->name);

			if (curnode->type == NODE_BRANCH) {
				char pos = gps_getnextswitchpos(curnode, nextedge);
				location switchloc = location_new(nextedge);
				trainvcmd_addswitch(rv_vcmd, &cmdlen, curnode->name, pos, &switchloc);
			}

			// reverse plan
			if (!nextedge) {
				location stoploc;
				gps_getstoploc(&stoploc, curnode, nextnode, train);

				trainvcmd_addstop(rv_vcmd, &cmdlen, &stoploc);
				trainvcmd_addpause(rv_vcmd, &cmdlen, REVERSE_TIMEOUT);
				trainvcmd_addreverse(rv_vcmd, &cmdlen, &stoploc);
				trainvcmd_addspeed(rv_vcmd, &cmdlen, CRUISE_SPEED, NULL);
			}
		}

		{
			track_node *last_node = path[pathlen - 1];
			if (last_node->type == NODE_BRANCH) {
				char pos = gps_getnextswitchpos(last_node, dest->edge);
				location switchloc = location_new(dest->edge);
				trainvcmd_addswitch(rv_vcmd, &cmdlen, last_node->name, pos, &switchloc);
			}
		}

		if (path[pathlen - 1] == trainloc.edge->dest) {
			track_node *cur_node = trainloc.edge->dest;
			track_node *next_node = trainloc.edge->dest->reverse;
			location stoploc;
			gps_getstoploc(&stoploc, cur_node, next_node, train);
			trainvcmd_addstop(rv_vcmd, &cmdlen, &stoploc);
			trainvcmd_addpause(rv_vcmd, &cmdlen, REVERSE_TIMEOUT);
			trainvcmd_addreverse(rv_vcmd, &cmdlen, &stoploc);
			trainvcmd_addspeed(rv_vcmd, &cmdlen, CRUISE_SPEED, NULL);
		}

//			ExitKernel(0);
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
void dijkstra(track_node *nodeary, heap *unoptimized, track_node *src, track_node *tgt, track_node **rv_nodes, int *rv_nodecnt, train_descriptor *train) {
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

	while (!heap_empty(unoptimized)) {
		track_node *u = heap_extract_min(unoptimized);
		int uidx = u - nodeary;

		if (dist[uidx] == infinity) {
			break;
		}

		if (u == tgt) {
			break;
		}

		{
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
		{
			track_node *u_rev = u->reverse;
			int u_rev_idx = u_rev - nodeary;
			int rev_dist = train_get_reverse_cost(train, dist[uidx]);
			int alt_rev_dist = (dist[u_rev_idx] == infinity) ? rev_dist : dist[u_rev_idx] + rev_dist;
			if (alt_rev_dist < dist[u_rev_idx]) {
				dist[u_rev_idx] = alt_rev_dist;
				previous[u_rev_idx] = u;
				heap_decrease_key_min(unoptimized, u_rev, alt_rev_dist);
			}
		}
	}

	if (dist[tgtidx] != infinity) {
		track_node *u = tgt;
		int uidx = u - nodeary;
		int pos = 0;

		while (u) {
			ASSERT(pos < MAX_PATH,  "path size too big!? %d", pos);
			rv_nodes[pos++] = u;
			u = previous[uidx];
			uidx = u - nodeary;
		}

		for (int i = 0; i < pos / 2; i++) { // reverse the path
			track_node *n = rv_nodes[i];
			rv_nodes[i] = rv_nodes[pos - i - 1];
			rv_nodes[pos - i - 1] = n;
		}

		*rv_nodecnt = pos;
	}
	else {
		*rv_nodecnt = 0;
	}
}

static fixed gps_distance(location *start, location *end, track_node **path, int pathlen) {
	ASSERT((int)path % 4 == 0, "path unaligned");
	ASSERT((int)start % 4 == 0, "start unaligned");
	ASSERT((int)end % 4 == 0, "end unaligned");
	fixed total = fixed_new(0);
	ASSERT(start->edge->dist > 0, "start->edge->dist %d", start->edge->dist);
	total = fixed_add(total, fixed_sub(fixed_new(start->edge->dist), start->offset));
	total = fixed_add(total, end->offset);
	int cnt = 0;
	char startbuf[100], endbuf[100];
	location_tostring(start, startbuf);
	location_tostring(end, endbuf);

	if (pathlen > 0) {
		track_node *curnode = path[cnt];
		while (curnode != end->edge->src) {
			track_edge * const curedge = gps_gettrackedge(curnode, path[cnt+1]);
			ASSERT(curedge || curnode->reverse == path[cnt+1], "wtf");
			if (curedge) { // this is a non reverse segment
				total = fixed_add(total, fixed_new(curedge->dist));
			}
			curnode = path[++cnt];
		}
	}

	return total;
}

int vcmd2str(char *buf, trainvcmd *vcmd) {
	char * const origbuf = buf;
	char locname[100];
	buf += sprintf(buf, "[%s ", vcmdnames[vcmd->name]);
	switch (vcmd->name) {
		case VCMD_WAITFORLOC:
			buf += sprintf(buf, "src:%s offset:%F", vcmd->location.edge->src, vcmd->location.offset);
			break;
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
