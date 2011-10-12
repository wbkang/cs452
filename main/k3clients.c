#include <k3clients.h>

#include <syscall.h>
#include <util.h>

#define PRINTF(...) { bwprintf(1, "[%d\t]\t", Time()); bwprintf(1, __VA_ARGS__); bwprintf(1, "\n");}

static inline void do_stuff(int name, int delay_time, int delays) {
	PRINTF("Task %d Entering", name);
	for(int i = 0; i < delays; i++) {
		PRINTF("Task %d Delay %d #%d", name, delay_time, i+1);
		Delay(delay_time);
		PRINTF("Task %d Returned from Delay", name);
	}
	Send(MyParentsTid(), NULL, 0, NULL, 0);
	PRINTF("Task %d Exiting", name);
}

static void t3() {
	do_stuff(3, 10, 20);
}

static void t4() {
	do_stuff(4, 23, 9);
}

static void t5() {
	do_stuff(5, 33, 6);
}

static void t6() {
	do_stuff(6, 71, 3);
}


void k3main() {
	PRINTF("Entering main");
	int t3tid = Create(6, t3);
	int t4tid = Create(5, t4);
	int t5tid = Create(4, t5);
	int t6tid = Create(3, t6);

	for (int i =0; i < 4; i++) {
		int tid;
		int retval;
		retval = Receive(&tid, NULL, 0);
		ASSERT(retval >= 0, "oops");
		Reply(tid, NULL, 0);
	}

	PRINTF("Exiting main");
}
