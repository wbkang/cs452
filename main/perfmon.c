#include <perfmon.h>
#include <syscall.h>
#include <constants.h>
#include <util.h>

#define MSG_SIZE 4
#define SENDER_FIRST 0
#define ITERATION 10000
#define SENDER_PRIORITY (MIN_PRIORITY + 1)
#define RECEIVER_PRIORITY SENDER_PRIORITY

static void receiver();

static void sender() {
	int rcvtid = Create(SENDER_PRIORITY + (SENDER_FIRST ? -1 : 1), receiver);
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
	int tid, msglen;
	char buf[MSG_SIZE];
	while(TRUE) {
		Receive(&tid, buf, MSG_SIZE);
		Reply(tid, buf, MSG_SIZE);
	}
}

void perfmon() {
	VMEM(TIMER3_BASE + CRTL_OFFSET) &= ~ENABLE_MASK;
	VMEM(TIMER3_BASE + LDR_OFFSET)= 0xffffffff;
	VMEM(TIMER3_BASE + CRTL_OFFSET) &= ~MODE_MASK;
	VMEM(TIMER3_BASE + CRTL_OFFSET) &= ~CLKSEL_MASK;
	VMEM(TIMER3_BASE + CRTL_OFFSET) |= ENABLE_MASK;
	Create(SENDER_PRIORITY, sender);
}
