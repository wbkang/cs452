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

#define LEN_MSG 1024

static lookup* ask_track(int tid_com2, track_node* data) {
	while (1) {
		Putstr(COM2, "Track a or b?\n", tid_com2);
		char c = Getc(COM2, tid_com2);
		if (c == 'a') {
			return  init_tracka(data);
		} else if (c == 'b') {
			return  init_trackb(data);
		} else {
			Putstr(COM2, "You suck.\n", tid_com2);
		}
	}
	return NULL;
}

static inline void handle_comin(msg_header *msg) {
	msg_comin *comin = (msg_comin*)msg;
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

	switch(orig->type) {
		case NODE_SENSOR:{
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

static void handle_sensor(msg_header *msg, lookup *sensormap, int tid_com2, int tid_time) {
	static uint last_tick = 0;
	static track_node *last_node = NULL;
	msg_sensor *sensor = (msg_sensor*)msg;

	char modname[4] = { 0 };
	modname[0] = sensor->module;
	sprintf(modname+1, "%d", sensor->id);

	track_node *cur_node = lookup_get(sensormap, modname);

	char msgbuf[1024], *b = msgbuf;
//	sprintf(msgbuf, "Sensor detected. realname:%c%d modname:%s nodename: %s\n",
//			sensor->module, sensor->id, modname, cur_node->name);
//	Putstr(COM2, msgbuf, tid_com2);

	if (last_node != NULL) {
		int dist = find_dist(last_node, cur_node, 0, 10);
		if (dist == -1) dist = find_dist(last_node->reverse, cur_node, 0, 10);
		sprintf(msgbuf, "%d\t%d\n", dist, Time(tid_time) - last_tick);
		Putstr(COM2, msgbuf, tid_com2);
	}

	last_tick = Time(tid_time);
	last_node = cur_node;
}

void calib_test() {
	int tid_time = WhoIs(NAME_TIMESERVER);
	int tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	int tid_com2 = WhoIs(NAME_IOSERVER_COM2);
	int tid_traincmdbuf = traincmdbuffer_new();
	track_node *track_data = malloc(sizeof(track_node) * TRACK_MAX);
	lookup *sensormap = ask_track(tid_com2, track_data);
	traincmdrunner_new();
	sensornotifier_new(MyTid());
	comnotifier_new(MyTid(), 10, COM2, tid_com2);

	for (;;) {
		int tid;
		char msg[LEN_MSG];
		int rcvlen = Receive(&tid, msg, LEN_MSG);
		ASSERT(rcvlen >= sizeof(msg_header), "bad data");
		Reply(tid, NULL, 0);
		msg_header *header = (msg_header*) msg;
		switch (header->type) {
			case SENSOR:
				handle_sensor(header, sensormap, tid_com2, tid_time);
				break;
			case COM_IN:
				handle_comin(header);
				break;
			default:
				break;
		}
	}
}
