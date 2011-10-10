#include <timeserver.h>
#include <syscall.h>
#include <heap.h>
#include <task.h>
#include <util.h>

typedef struct _tag_timeserver_req {
	char no;
	int ticks;
} timeserver_req;

typedef struct _tag_timeserver_state {
	heap *tasks;
	uint time; // in ticks, 1 tick = 10ms, will overflow every (2^32-1)*10 ms = 1.36 years
} timeserver_state;

inline void unblock(int tid, int rv) {
	Reply(tid, (void*) &rv, sizeof rv);
}

inline void timeserver_do_tick(timeserver_state *state) {
	// grab the time
	state->time += ~VMEM(TIMER3_BASE + VAL_OFFSET) / 20; // # of 10ms
	VMEM(TIMER3_BASE + LDR_OFFSET) = ~0;
	// unblock waiting tasks
	for (;;) {
		heap_item *item = heap_peek(state->tasks);
		if (item == NULL || item->key > state->time) break; // rest of tasks must wait longer
		int tid = (int) heap_extract_min(state->tasks);
		unblock(tid, 0);
	}
}

inline void timeserver_do_time(timeserver_state *state, int tid) {
	unblock(tid, state->time);
}

inline void timeserver_do_delayuntil(timeserver_state *state, int tid, int ticks) {
	if (ticks < 0) ticks = 0;
	heap_insert_max(state->tasks, (void*) tid, ticks);
}

void timeserver() {
	RegisterAs(TIMESERVER_NAME);
	// init timer 3
	VMEM(TIMER3_BASE + CRTL_OFFSET) &= ~ENABLE_MASK; // stop timer
	VMEM(TIMER3_BASE + LDR_OFFSET) = ~0;
	VMEM(TIMER3_BASE + CRTL_OFFSET) &= ~MODE_MASK; // free run mode
	VMEM(TIMER3_BASE + CRTL_OFFSET) &= ~CLKSEL_MASK; // 2k clock
	VMEM(TIMER3_BASE + CRTL_OFFSET) |= ENABLE_MASK; // start
	// init state
	timeserver_state state;
	state.tasks = heap_new(TASK_LIST_SIZE);
	state.time = 0;
	// init com arguments
	int tid;
	timeserver_req req;
	// serve
	for (;;) {
		int msglen = Receive(&tid, (void*) &req, sizeof(req));
		if (msglen == sizeof(req)) {
			switch (req.no) {
				case TIMESERVER_TICK:
					timeserver_do_tick(&state);
					break;
				case TIMESERVER_TIME:
					timeserver_do_time(&state, tid);
					break;
				case TIMESERVER_DELAYUNTIL:
					timeserver_do_delayuntil(&state, tid, req.ticks);
					break;
				default:
					unblock(tid, TIMESERVER_ERROR_BADREQNO);
			}
		} else {
			unblock(tid, TIMESERVER_ERROR_BADDATA);
		}
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