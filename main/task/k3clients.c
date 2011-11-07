#include <syscall.h>
#include <util.h>
#include <stdio.h>
#include <task/k3clients.h>

typedef struct {
	int delay_time;
	int delays;
} delay_info;

#define PRINTF(comserver, timeserver, ...) { \
	char __buf[100], *__p = __buf; \
	__p += sprintf(__p, "[%d\t] ", Time(timeserver)); \
	__p += sprintf(__p, __VA_ARGS__); \
	__p += sprintf(__p, "\n"); Putstr(1, __buf, comserver); \
}

#define SAY(comserver, timeserver, name, ...) { \
	char __buf[1000], *__p = __buf; \
	__p += sprintf(__buf, "[%d\t] ", Time(timeserver)); \
	for (int i = 0; i < 2 * (name - 8); i++) { __p += sprintf(__buf, "\t"); } \
	__p += sprintf(__buf, "{%d} ", name); \
	__p += sprintf(__buf, __VA_ARGS__); \
	__p += sprintf(__buf, "\n"); Putstr(1, __buf, comserver); \
}

static inline void client_task() {
	delay_info arg;
	Send(MyParentsTid(), NULL, 0, &arg, sizeof arg);
	int mytid = MyTid();
	int delay_time = arg.delay_time;
	int delays = arg.delays;

	int timeserver = WhoIs(NAME_TIMESERVER);
	int com2server = WhoIs(NAME_IOSERVER_COM2);
	SAY(com2server, timeserver, mytid, "started");
	for(int i = 0; i < delays; i++) {
		SAY(com2server, timeserver, mytid, "delay %d for %d ticks", i + 1, delay_time);
		Delay(delay_time, timeserver);
		SAY(com2server, timeserver, mytid, "back from delay %d", i + 1);
	}
	SAY(com2server, timeserver, mytid, "exited");
	Send(MyParentsTid(), NULL, 0, NULL, 0);
}

void k3main() {
	int timeserver = WhoIs(NAME_TIMESERVER);
	int com2server = WhoIs(NAME_IOSERVER_COM2);
	PRINTF(com2server, timeserver, "Entering main");

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
		Receive(&tid, NULL, 0);
		Reply(tid, NULL, 0);
	}

	PRINTF(com2server, timeserver, "Exiting main");
	ExitKernel(0);
}
