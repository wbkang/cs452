#include <timeserver.h>
#include <syscall.h>
#include <heap.h>
#include <task.h>

typedef struct _tag_timeserver_req {
	char no;
	int ticks;
} timeserver_req;

typedef struct _tag_timeserver_state {
	heap *tasks;
	uint time; // in ticks, 1 tick = 50ms
} timeserver_state;

inline int timeserver_do_tick(timeserver_state *state) {
	// grab the time
	// state->time = ...
	// unblock waiting tasks
	int rv = 0;
	for (;;) {
		heap_item *item = heap_peek(state->tasks);
		if (item == NULL || item->key > state->time) return 0;
		int tid = (int) heap_extract_min(state->tasks);
		Reply(tid, (void*) &rv, sizeof rv); // unblock and return 0
	}
}

inline int timeserver_do_time(timeserver_state *state) {
	return state->time;
}

inline int timeserver_do_delayuntil(timeserver_state *state, int tid, int ticks) {
	if (ticks < 0) ticks = 0;
	heap_insert_max(state->tasks, (void*) tid, ticks);
	return TIMESERVER_DONTUNBLOCK;
}

inline int timeserver_switchboard(timeserver_state *state, int tid, timeserver_req *req) {
	switch (req->no) {
		case TIMESERVER_TICK: return timeserver_do_tick(state);
		case TIMESERVER_TIME: return timeserver_do_time(state);
		case TIMESERVER_DELAYUNTIL: return timeserver_do_delayuntil(state, tid, req->ticks);
		default: return TIMESERVER_ERROR_BADREQNO;
	}
}

void timeserver() {
	RegisterAs(TIMESERVER_NAME);
	// init state
	timeserver_state state;
	state.tasks = heap_new(TASK_LIST_SIZE);
	// init com arguments
	int tid;
	timeserver_req req;
	int rv;
	// serve
	for (;;) {
		int msglen = Receive(&tid, (void*) &req, sizeof(req));
		if (msglen == sizeof(req)) {
			rv = timeserver_switchboard(&state, tid, &req);
		} else {
			rv = TIMESERVER_ERROR_BADDATA;
		}
		if (rv <= 0) Reply(tid, (void*) &rv, sizeof rv); // return error / normal exit
	}
}

inline int timeserver_send(timeserver_req *req) {
	int server = WhoIs(TIMESERVER_NAME);
	if (server < 0) return server;
	int rv;
	int len = Send(server, (void*) req, sizeof(timeserver_req), (void*) &rv, sizeof rv);
	if (len != sizeof rv) return TIMESERVER_ERROR_BADDATA;
	return rv;
}

/*
 * API
 */

inline int timeserver_tick() {
	timeserver_req req;
	req.no = TIMESERVER_TICK;
	return timeserver_send(&req);
}

inline int timeserver_time() {
	timeserver_req req;
	req.no = TIMESERVER_TIME;
	return timeserver_send(&req);
}

inline int timeserver_delay(int ticks) {
	int time = timeserver_time();
	if (time < 0) return time;
	return timeserver_delayuntil(time + ticks);
}

inline int timeserver_delayuntil(int ticks) {
	timeserver_req req;
	req.no = TIMESERVER_DELAYUNTIL;
	req.ticks = ticks;
	return timeserver_send(&req);
}
