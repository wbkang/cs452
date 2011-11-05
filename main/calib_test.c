#include <calib_test.h>
#include <train.h>
#include <syscall.h>
#include <sensornotifier.h>
#include <uconst.h>
#include <comnotifier.h>
#include <string.h>
#include <traincmdbuffer.h>
#include <traincmdrunner.h>
#include <stdio.h>
#include <timenotifier.h>
#include <track_node.h>
#include <track_data.h>
#include <lookup.h>
#include <console.h>

#define LEN_MSG 1024

typedef struct {
	int tid_time;
	int tid_com1;
	int tid_com2;
	int tid_traincmdbuf;
	// sensors
	uint last_tick;
	track_node *last_node;
} calib_state;

static lookup *ask_track(int tid_com2, track_node* data) {
	for (;;) {
		Putstr(COM2, "Track a or b?\n", tid_com2);
		char c = Getc(COM2, tid_com2);
		switch (c) {
			case 'a': return init_tracka(data);
			case 'b': return init_trackb(data);
			default: Putstr(COM2, "fail\n", tid_com2);
		}
	}
	return NULL;
}

static inline void handle_comin(calib_state *state, msg_header *msg) {
	msg_comin *comin = (msg_comin*) msg;
	if (comin->c == 'q') {
		ExitKernel(1);
	}
}

static int find_dist(track_node *orig, track_node *dest, int curdist, int maxdepth) {
	//	NODE_SENSOR, NODE_BRANCH, NODE_MERGE, NODE_ENTER, NODE_EXIT
	if (dest == orig) {
		return curdist;
	} else if (maxdepth == 0) {
		return -1;
	}

	switch (orig->type) {
		case NODE_SENSOR: {
			return find_dist(orig->edge[0].dest, dest, curdist + orig->edge[0].dist, maxdepth - 1);
		}
		case NODE_BRANCH: {
			int dist = find_dist(orig->edge[0].dest, dest, curdist + orig->edge[0].dist, maxdepth - 1);
			if (dist != -1) {
				return dist;
			} else {
				return find_dist(orig->edge[1].dest, dest, curdist + orig->edge[1].dist, maxdepth - 1);
			}
		}
		case NODE_MERGE: {
			return find_dist(orig->edge[0].dest, dest, curdist + orig->edge[0].dist, maxdepth - 1);
		}
		default:
			return -1;
	}
}

static void handle_sensor(calib_state *state, msg_header *msg, lookup *sensormap) {
	msg_sensor *sensor = (msg_sensor*) msg;

	char modname[8];
	sprintf(modname, "%c%d", sensor->module, sensor->id);
	track_node *cur_node = lookup_get(sensormap, modname);

	char msgbuf[1024];
//	char *b = msgbuf;
//	sprintf(msgbuf, "Sensor detected. realname:%c%d modname:%s nodename: %s\n",
//			sensor->module, sensor->id, modname, cur_node->name);
//	Putstr(COM2, msgbuf, tid_com2);

	if (state->last_node != NULL && strcmp(cur_node->name, "C14") == 0) {
		int dist = find_dist(state->last_node, cur_node, 0, 10);
		if (dist == -1) dist = find_dist(state->last_node->reverse, cur_node, 0, 10);
		int dt = Time(state->tid_time) - state->last_tick;
		int speed = (1000 * dist) / dt;
		sprintf(msgbuf, "%s\t%s\t%d\t%d\t%d.%d\n",
			state->last_node->name,
			cur_node->name,
			dist,
			dt,
			speed / 1000,
			speed % 1000
		);
		Putstr(COM2, msgbuf, state->tid_com2);
	}

	state->last_tick = Time(state->tid_time);
	state->last_node = cur_node;
}

void calib_test() {
	calib_state state;
	state.tid_time = WhoIs(NAME_TIMESERVER);
	state.tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	state.tid_com2 = WhoIs(NAME_IOSERVER_COM2);
	state.tid_com2 = WhoIs(NAME_IOSERVER_COM2);
	state.tid_traincmdbuf = traincmdbuffer_new();
	state.last_tick = 0;
	state.last_node = NULL;

	track_node *track_data = malloc(sizeof(track_node) * TRACK_MAX);
	lookup *sensormap = ask_track(state.tid_com2, track_data);
	traincmdrunner_new();
	sensornotifier_new(MyTid());
	comnotifier_new(MyTid(), 10, COM2, state.tid_com2);

	char buf[16];
	console_clear(buf);
	Putstr(COM2, buf, state.tid_com2);

	for (;;) {
		int tid;
		char msg[LEN_MSG];
		int rcvlen = Receive(&tid, msg, LEN_MSG);
		ASSERT(rcvlen >= sizeof(msg_header), "bad data");
		Reply(tid, NULL, 0);
		msg_header *header = (msg_header*) msg;
		switch (header->type) {
			case SENSOR:
				handle_sensor(&state, header, sensormap);
				break;
			case COM_IN:
				handle_comin(&state, header);
				break;
			default:
				break;
		}
	}
}
