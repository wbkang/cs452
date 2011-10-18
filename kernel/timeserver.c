#include <timeserver.h>
#include <notifier.h>
#include <syscall.h>
#include <heap.h>
#include <task.h>
#include <util.h>
#include <constants.h>
#include <ts7200.h>

typedef struct _tag_timeserver_req {
	int no;
	int ticks;
} timeserver_req;

typedef struct _tag_timeserver_state {
	uint time; // in ticks, 1 tick = 10ms, will overflow every (2^32-1)*10 ms = 1.36 years
	heap *tasks;
} timeserver_state;

inline void timeserver_do_tick(timeserver_state *state, int tid_notifier) {
	VMEM(TIMER1_BASE + CLR_OFFSET) = 1; // clear interrupt source
	ReplyInt(tid_notifier, 0); // unblock notifier
	state->time++;
	for (;;) { // unblock waiting tasks
		heap_item *item = heap_peek(state->tasks);
		if (item == NULL || item->key > state->time) break;
		int tid = (int) heap_extract_min(state->tasks);
		ReplyInt(tid, 0);
	}
}

inline void timeserver_do_time(timeserver_state *state, int tid) {
	ReplyInt(tid, state->time);
}

inline void timeserver_do_delayuntil(timeserver_state *state, int tid, int ticks) {
	heap_insert_min(state->tasks, (void*) tid, ticks);
}

inline void timeserver_do_delay(timeserver_state *state, int tid, int ticks) {
	timeserver_do_delayuntil(state, tid, state->time + ticks);
}

void timeserver() {
	RegisterAs(NAME_TIMESERVER);
	// init timer 3
	VMEM(TIMER1_BASE + CRTL_OFFSET) &= ~ENABLE_MASK; // stop timer
	VMEM(TIMER1_BASE + LDR_OFFSET) = 508 * TIMESERVER_RATE;
	VMEM(TIMER1_BASE + CRTL_OFFSET) |= MODE_MASK; // pre-load mode
	VMEM(TIMER1_BASE + CRTL_OFFSET) |= CLKSEL_MASK; // 508k clock
	VMEM(TIMER1_BASE + CRTL_OFFSET) |= ENABLE_MASK; // start
	// init state
	timeserver_state state;
	state.tasks = heap_new(TASK_LIST_SIZE);
	ASSERT(state.tasks, "no memory");
	state.time = 0;
	// init com arguments
	timeserver_req req;
	int tid;
	// init notifier
	int tid_notifier = notifier_new(PRIORITY_TIMENOTIFIER, EVENT_TIMER1);
	Receive(&tid, NULL, 0);
	ASSERT(tid == tid_notifier, "got a message during initialization from %d", tid);
	Reply(tid_notifier, NULL, 0);
	// serve
	for (;;) {
		ASSERT(state.tasks, "state->tasks is invalid");
		int msglen = Receive(&tid, (void*) &req, sizeof(req));
		if (tid == tid_notifier) {
			timeserver_do_tick(&state, tid);
		} else if (msglen == sizeof(req)) {
			switch (req.no) {
				case TIMESERVER_TIME:
					timeserver_do_time(&state, tid);
					break;
				case TIMESERVER_DELAYUNTIL:
					timeserver_do_delayuntil(&state, tid, req.ticks);
					break;
				case TIMESERVER_DELAY:
					timeserver_do_delay(&state, tid, req.ticks);
					break;
				default:
					ASSERT(FALSE, "bad reqno");
					ReplyInt(tid, TIMESERVER_ERROR_BADREQNO);
			}
		} else {
			ASSERT(FALSE, "bad data");
			ReplyInt(tid, TIMESERVER_ERROR_BADDATA);
		}
	}
}

inline int timeserver_send(timeserver_req *req, int server) {
	int rv;
	int len = Send(server, (void*) req, sizeof(timeserver_req), (void*) &rv, sizeof rv);
	if (len != sizeof rv) return TIMESERVER_ERROR_BADDATA;
	return rv;
}

/*
 * API
 */

inline int timeserver_time(int timeserver) {
	timeserver_req req;
	req.no = TIMESERVER_TIME;
	return timeserver_send(&req, timeserver);
}

inline int timeserver_delayuntil(int ticks, int timeserver) {
	ASSERT(ticks > 0, "negative ticks");
	timeserver_req req;
	req.no = TIMESERVER_DELAYUNTIL;
	req.ticks = ticks;
	return timeserver_send(&req, timeserver);
}

inline int timeserver_delay(int ticks, int timeserver) {
	ASSERT(ticks > 0, "negative ticks");
	timeserver_req req;
	req.no = TIMESERVER_DELAY;
	req.ticks = ticks;
	return timeserver_send(&req, timeserver);
}
