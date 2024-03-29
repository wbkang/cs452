#include <syscall.h>
#include <constants.h>
#include <util.h>
#include <task/perfmon.h>

#define MSG_SIZE 64
#define SENDER_FIRST 0

#define ITERATION 10000
#define SENDER_PRIORITY (MAX_PRIORITY)
#define RECEIVER_PRIORITY SENDER_PRIORITY

static void setup_timer() {
	VMEM(TIMER3_BASE + CRTL_OFFSET) &= ~ENABLE_MASK; // stop timer
	VMEM(TIMER3_BASE + LDR_OFFSET) = ~0;
	VMEM(TIMER3_BASE + CRTL_OFFSET) &= ~MODE_MASK; // free-running mode
	VMEM(TIMER3_BASE + CRTL_OFFSET) |= CLKSEL_MASK; // 508Khz clock
	VMEM(TIMER3_BASE + CRTL_OFFSET) |= ENABLE_MASK; // start
}

static uint timer() {
	return ~VMEM(TIMER3_BASE + VAL_OFFSET) & 0xffffffff;
}

static void receiver();

static void sender() {
	PRINT("Sender test starting");
	int rcvtid = Create(SENDER_PRIORITY + (SENDER_FIRST ? -1 : 0), receiver);
	ASSERT(rcvtid >= 0, "oops");
	char buf[MSG_SIZE];
	for (int i = 0; i < MSG_SIZE; i++) {
		buf[i] = '0' + (i % 10);
	}
	buf[MSG_SIZE - 1] = '\0';
	char replybuf[MSG_SIZE];
	uint start_time = timer();
	for (int i = ITERATION; i > 0; --i) {
		Send(rcvtid, buf, MSG_SIZE, replybuf, MSG_SIZE);
	}
	uint end_time = timer();
	int elapsed_time = end_time - start_time;
	int per1000 = (1000 * elapsed_time) / ITERATION;
	PRINT("START TIME: %d, END TIME: %d", start_time, end_time);
	PRINT("Time elapsed: %d.%d ticks per iteration. %d iterations in total.",
			per1000 / 1000, per1000 % 1000, ITERATION);
	ExitKernel(0);
}

static void receiver() {
	int tid;
	int buf[(MSG_SIZE / sizeof(int)) + 1];
	for (;;) {
		Receive(&tid, buf, MSG_SIZE);
		Reply(tid, buf, MSG_SIZE);
	}
}

/*
static void context_switch() {
	uint start_time;
	uint end_time;
	PRINT("ctxswtch test starting");
	start_time = timer();
	for (int i =0; i < 10000; i++) {
		Pass();
	}
	end_time = timer();
	int elapsed_time = end_time - start_time;
	PRINT("START TIME: %d, END TIME: %d, diff: %d", start_time, end_time, elapsed_time);
//		PRINT("Time elapsed: %dms. %d per iteration. %d iterations in total.",
//				elapsed_time, elapsed_time / ITERATION, ITERATION);
}
*/

void perfmon() {
	setup_timer();
	Create(MAX_PRIORITY, sender);
//	Create(MIN_PRIORITY, sender);
//	Create(MIN_PRIORITY, context_switch);
//	Create(MAX_PRIORITY, context_switch);
//	Create(MIN_PRIORITY, context_switch);
}
