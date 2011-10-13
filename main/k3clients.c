#include <k3clients.h>

#include <syscall.h>
#include <util.h>

#define PRINTF(...) { \
	bwprintf(1, "[%d\t] ", Time()); \
	bwprintf(1, __VA_ARGS__); \
	bwprintf(1, "\n"); \
}

#define SAY(name, ...) { \
	bwprintf(1, "[%d\t] ", Time()); \
	for (int i = 0; i < 2 * (name - 3); i++) bwprintf(1, "\t"); \
	bwprintf(1, "{%d} ", name); \
	bwprintf(1, __VA_ARGS__); \
	bwprintf(1, "\n"); \
}

static inline void do_stuff(int name, int delay_time, int delays) {
	SAY(name, "started");
	for(int i = 0; i < delays; i++) {
		SAY(name, "delay %d for %d ticks", i + 1, delay_time);
		Delay(delay_time);
		SAY(name, "back from delay %d", i + 1);
	}
	Send(MyParentsTid(), NULL, 0, NULL, 0);
	SAY(name, "exited");
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
	Create(6, t3);
	Create(5, t4);
	Create(4, t5);
	Create(3, t6);

	for (int i = 0; i < 4; i++) {
		int tid;
		int rv = Receive(&tid, NULL, 0);
		ASSERT(rv >= 0, "oops");
		Reply(tid, NULL, 0);
	}

	PRINTF("Exiting main");
}
