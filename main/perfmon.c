#include <perfmon.h>
#include <syscall.h>
#include <constants.h>
#include <util.h>

#define MSG_SIZE 64
#define SENDER_FIRST 0

#define ITERATION 10000
#define SENDER_PRIORITY (MAX_PRIORITY)
#define RECEIVER_PRIORITY SENDER_PRIORITY

static void receiver();

static void sender() {
	int rcvtid = Create(SENDER_PRIORITY + (SENDER_FIRST ? -1 : 0), receiver);
	char buf[MSG_SIZE]
#if MSG_SIZE == 64
	 = "012345678901234567890123456789012345678901234567890123456789123";
#else
	= "123";
#endif
	char replybuf[MSG_SIZE];

	uint start_time;
	uint end_time;
	PRINT("Sender test starting");
	GET_TIME(start_time);

	for ( int i = 0 ; i < ITERATION; i++) {
		Send(rcvtid, buf, MSG_SIZE, replybuf, MSG_SIZE);
	}

	GET_TIME(end_time);

	int elapsed_time = end_time - start_time;
	PRINT("START TIME: %d, END TIME: %d", start_time, end_time);
	PRINT("Time elapsed: %dms. %d per iteration. %d iterations in total.",
			elapsed_time, elapsed_time / ITERATION, ITERATION);
}

static void receiver() {
	int tid;
	char buf[MSG_SIZE];
	while(TRUE) {
		Receive(&tid, buf, MSG_SIZE);
		Reply(tid, buf, MSG_SIZE);
	}
}

static void setup_timer() {
	VMEM(TIMER3_BASE + CRTL_OFFSET) &= ~ENABLE_MASK;
	VMEM(TIMER3_BASE + LDR_OFFSET)= 0xffffffff;
	VMEM(TIMER3_BASE + CRTL_OFFSET) &= ~MODE_MASK;
	VMEM(TIMER3_BASE + CRTL_OFFSET) |= CLKSEL_MASK;
	VMEM(TIMER3_BASE + CRTL_OFFSET) |= ENABLE_MASK;
}

static void context_switch() {
	uint start_time;
	uint end_time;
	PRINT("ctxswtch test starting");
	GET_TIME(start_time);
	for (int i =0; i<10000; i++) {
		Pass();
	}
	GET_TIME(end_time);

	int elapsed_time = end_time - start_time;
	PRINT("START TIME: %d, END TIME: %d, diff: %d", start_time, end_time, elapsed_time);
//		PRINT("Time elapsed: %dms. %d per iteration. %d iterations in total.",
//				elapsed_time, elapsed_time / ITERATION, ITERATION);
}
void perfmon() {
	setup_timer();
	Create(MAX_PRIORITY, sender);
//	Create(MIN_PRIORITY, sender);
//	Create(MIN_PRIORITY, context_switch);
//	Create(MAX_PRIORITY, context_switch);
//	Create(MIN_PRIORITY, context_switch);
}
