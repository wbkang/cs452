#include <task1.h>
#include <syscall.h>
#include <rpsserver.h>
#include <rpsclient.h>
#include <perfmon.h>

void task1() {
	Create(MAX_PRIORITY - 2, perfmon);
	//for (int i = 0; i < 2; i++) Create(0, rps_client);
}
