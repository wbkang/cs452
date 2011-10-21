#include <task1.h>
#include <syscall.h>
#include <timeserver.h>
#include <perfmon.h>
#include <k3clients.h>
#include <ioserver.h>
#include <a0.h>

#define RPS_SERVER 0
#define PERFMON 0
#define K3 0
#define A0 1

void task1() {
	int tid_time = Create(PRIORITY_TIMESERVER, timeserver);
	ioserver_create(COM1, OFF, 2400, 2, 8, OFF);
	ioserver_create(COM2, OFF, 115200, 1, 8, OFF);

#if RPS_SERVER
	Create(1, rps_server);
	for (int i = 0; i < 4; i++) Create(1, rps_client);
#endif

#if PERFMON
	Create(1, perfmon);
#endif

#if K3
	Create(10, k3main);
#endif

#if A0
	Create(1, a0);
#endif

	Delay(1000, tid_time);
	ExitKernel(0);
}
