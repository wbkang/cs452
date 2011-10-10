#include <task1.h>
#include <syscall.h>
#include <rpsserver.h>
#include <rpsclient.h>
#include <perfmon.h>

#define RPS_SERVER 0
#define PERFMON 1

void task1() {
#if RPS_SERVER
	Create(1, rps_server);
	for (int i = 0; i < 4; i++) Create(1, rps_client);
#endif

#if PERFMON
	Create(1, perfmon);
#endif
}
