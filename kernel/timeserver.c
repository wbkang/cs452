#include <timeserver.h>
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
	heap *tasks;
	uint time; // in ticks, 1 tick = 10ms, will overflow every (2^32-1)*10 ms = 1.36 years
} timeserver_state;

inline void unblock(int tid, int rv) {
	Reply(tid, (void*) &rv, sizeof rv);
}

static inline int timeserver_tick();

void timenotifier() {
	int timeserver = WhoIs(NAME_TIMESERVER);
	ASSERT(timeserver >= 0, "cant find time server");
	Send(timeserver, NULL, 0, NULL, 0);
	for (;;) {
		AwaitEvent(TC1UI);
		VMEM(TIMER1_BASE + CLR_OFFSET) = 1; // clear hardware interrupt status
		timeserver_tick(timeserver);
	}
}

inline void timeserver_do_tick(timeserver_state *state, int tid) {
	unblock(tid, 0); // unblock notifier
	state->time++;
	for (;;) { // unblock waiting tasks
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
	heap_insert_min(state->tasks, (void*) tid, ticks);
}

inline void timeserver_do_delay(timeserver_state *state, int tid, int ticks) {
	timeserver_do_delayuntil(state, tid, state->time + ticks);
}

void timeserver() {
	RegisterAs(NAME_TIMESERVER);
	int notifier = Create(PRIORITY_TIMENOTIFIER, timenotifier);
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
	int tid;
	timeserver_req req;
	// sync up with notifier
	do {
		Receive(&tid, NULL, 0);
		ASSERT(tid == notifier, "no the notifier %d (%x)", tid, tid);
		Reply(tid, NULL, 0);
	} while (tid != notifier);
	// serve
	for (;;) {
		ASSERT(state.tasks, "state->tasks is invalid");
		int msglen = Receive(&tid, (void*) &req, sizeof(req));
		if (msglen == sizeof(req)) {
			switch (req.no) {
				case TIMESERVER_TICK:
					timeserver_do_tick(&state, tid);
					break;
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
					unblock(tid, TIMESERVER_ERROR_BADREQNO);
			}
		} else {
			ASSERT(FALSE, "bad data");
			unblock(tid, TIMESERVER_ERROR_BADDATA);
		}
	}
}

inline int timeserver_send(timeserver_req *req, int server) {
	int rv;
	int len = Send(server, (void*) req, sizeof(timeserver_req), (void*) &rv, sizeof rv);
	if (len != sizeof rv) return TIMESERVER_ERROR_BADDATA;
	return rv;
}

static inline int timeserver_tick(int timeserver) {
	timeserver_req req;
	req.no = TIMESERVER_TICK;
	return timeserver_send(&req, timeserver);
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
