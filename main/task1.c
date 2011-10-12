#include <task1.h>
#include <syscall.h>
#include <rpsserver.h>
#include <rpsclient.h>
#include <perfmon.h>

#define RPS_SERVER 0
#define PERFMON 1
#define INT_TEST 0

void task1() {
#if RPS_SERVER
	Create(1, rps_server);
	for (int i = 0; i < 4; i++) Create(1, rps_client);
#endif

#if PERFMON
	Create(1, perfmon);
#endif

#if INT_TEST
	// this turns on software interrupt #5 and enables interrupt #5
	VMEM(VIC1 + SOFTINT_OFFSET) = 0x10;
	VMEM(VIC1 + INTENABLE_OFFSET) = 0x10;
	PRINT("RETURNED FROM INTERRUPT");
#endif
}
