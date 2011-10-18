#include <task1.h>
#include <syscall.h>
#include <timeserver.h>
#include <perfmon.h>
#include <k3clients.h>
#include <ioserver.h>

#define RPS_SERVER 0
#define PERFMON 0
#define K3 1

void task1() {
	Create(PRIORITY_TIMESERVER, timeserver);
//	ioserver_create(COM1, OFF, 2400, 2, 8, OFF);
	ioserver_create(COM2, OFF, 115200, 1, 8, OFF);

//	for(int i = 0; i < 10; i ++) { Putc(COM2, 'A'); }

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
}
