#include <k3clients.h>

#include <syscall.h>
#include <util.h>

typedef struct {
	int delay_time;
	int delays;
} delay_info;

#define PRINTF(timeserver, ...) { \
	bwprintf(1, "[%d\t] ", Time(timeserver)); \
	bwprintf(1, __VA_ARGS__); \
	bwprintf(1, "\n"); \
}

#define SAY(timeserver, name, ...) { \
	bwprintf(1, "[%d\t] ", Time(timeserver)); \
	for (int i = 0; i < 2 * (name - 6); i++) bwprintf(1, "\t"); \
	bwprintf(1, "{%d} ", name); \
	bwprintf(1, __VA_ARGS__); \
	bwprintf(1, "\n"); \
}

static inline void client_task() {
	delay_info arg;
	Send(MyParentsTid(), NULL, 0, (void*) &arg, sizeof arg);
	int mytid = MyTid();
	int delay_time = arg.delay_time;
	int delays = arg.delays;

	int timeserver = WhoIs(NAME_TIMESERVER);
	SAY(timeserver, mytid, "started");
	for(int i = 0; i < delays; i++) {
		SAY(timeserver, mytid, "delay %d for %d ticks", i + 1, delay_time);
		Delay(delay_time, timeserver);
		SAY(timeserver, mytid, "back from delay %d", i + 1);
	}
	SAY(timeserver, mytid, "exited");
	Send(MyParentsTid(), NULL, 0, NULL, 0);
}

void k3main() {
	int timeserver = WhoIs(NAME_TIMESERVER);
	PRINTF(timeserver, "Entering main");

	int delayinfo[4][3] = {
			{6, 10, 20}, {5, 23, 9}, {4, 33, 6}, {3, 71, 3}
	};

	for (int i = 0; i < 4; i++) {
		int tid;
		delay_info info;
		Create(delayinfo[i][0], client_task);
		Receive(&tid, NULL, 0);
		info.delay_time = delayinfo[i][1];
		info.delays = delayinfo[i][2];
		Reply(tid, &info, sizeof info);
	}

	for (int i = 0; i < 4; i++) {
		int tid;
		int rv = Receive(&tid, NULL, 0);
		ASSERT(rv >= 0, "oops");
		Reply(tid, NULL, 0);
	}

	PRINTF(timeserver, "Exiting main");
	ExitKernel(0);
}
