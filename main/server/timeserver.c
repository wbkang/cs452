#include <syscall.h>
#include <heap.h>
#include <util.h>
#include <constants.h>
#include <ts7200.h>
#include <uconst.h>
#include <server/timeserver.h>
#include <server/eventnotifier.h>

#define TIMESERVER_CAPACITY 512

typedef struct _tag_timeserver_req {
	int no;
	int ticks;
} timeserver_req;

typedef struct _tag_timeserver_state {
	uint time; // in ticks, 1 tick = 10ms, will overflow every (2^32-1)*10 ms = 1.36 years
	heap *tasks;
} timeserver_state;

static inline void timeserver_do_tick(timeserver_state *state, int tid_notifier) {
	VMEM(TIMER1_BASE + CLR_OFFSET) = 1; // clear interrupt source
	ReplyInt(tid_notifier, 0); // unblock notifier
	state->time++;
	while (!heap_empty(state->tasks) && heap_peekkey(state->tasks) <= state->time) {
		int tid = (int) heap_extract_min(state->tasks);
		ReplyInt(tid, 0);
	}
}

static inline void timeserver_do_time(timeserver_state *state, int tid) {
	ReplyInt(tid, state->time);
}

static inline void timeserver_do_delayuntil(timeserver_state *state, int tid, int ticks) {
	heap_insert_min(state->tasks, (void*) tid, ticks);
}

static inline void timeserver_do_delay(timeserver_state *state, int tid, int ticks) {
	timeserver_do_delayuntil(state, tid, state->time + ticks);
}

void timeserver() {
	RegisterAs(NAME_TIMESERVER);
	// init timer 3
	VMEM(TIMER1_BASE + CRTL_OFFSET) &= ~ENABLE_MASK; // stop timer
	VMEM(TIMER1_BASE + LDR_OFFSET) = TICK2MS(508);
	VMEM(TIMER1_BASE + CRTL_OFFSET) |= MODE_MASK; // pre-load mode
	VMEM(TIMER1_BASE + CRTL_OFFSET) |= CLKSEL_MASK; // 508k clock
	VMEM(TIMER1_BASE + CRTL_OFFSET) |= ENABLE_MASK; // start
	// init state
	timeserver_state state;
	state.tasks = heap_new(TIMESERVER_CAPACITY);
	ASSERT(state.tasks, "no memory");
	state.time = 0;
	// init com arguments
	timeserver_req req;
	int tid;
	// init notifier
	int tid_notifier = eventnotifier_new(PRIORITY_TIMENOTIFIER, EVENT_TIMER1);
	Receive(&tid, NULL, 0);
	ASSERT(tid == tid_notifier, "got a message during initialization from %d", tid);
	Reply(tid_notifier, NULL, 0);
	// serve
	for (;;) {
		ASSERT(state.tasks, "state->tasks is invalid");
		int msglen = Receive(&tid, &req, sizeof(req));
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

static inline int timeserver_send(timeserver_req *req, int server) {
	int rv;
	int len = Send(server, req, sizeof(timeserver_req), &rv, sizeof rv);
	if (len != sizeof rv) return TIMESERVER_ERROR_BADDATA;
	return rv;
}

/*
 * API
 */

int timeserver_create() {
	return Create(PRIORITY_TIMESERVER, timeserver);
}

int Time(int timeserver) {
	timeserver_req req;
	req.no = TIMESERVER_TIME;
	return timeserver_send(&req, timeserver);
}

int Delay(int ticks, int tid) {
	ASSERT(ticks >= 0, "negative ticks %d", ticks);
	timeserver_req req;
	req.no = TIMESERVER_DELAY;
	req.ticks = ticks;
	return timeserver_send(&req, tid);
}

int DelayUntil(int ticks, int tid) {
	ASSERT(ticks > 0, "negative ticks");
	timeserver_req req;
	req.no = TIMESERVER_DELAYUNTIL;
	req.ticks = ticks;
	return timeserver_send(&req, tid);
}
