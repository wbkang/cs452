#include <task1.h>
#include <syscall.h>
#include <timeserver.h>
#include <perfmon.h>

#define RPS_SERVER 0
#define PERFMON 0

void task1() {
	Create(PRIORITY_TIMESERVER, timeserver);


#if RPS_SERVER
	Create(1, rps_server);
	for (int i = 0; i < 4; i++) Create(1, rps_client);
#endif

#if PERFMON
	Create(1, perfmon);
#endif

	for (;;) bwputc(1, '.');
}
