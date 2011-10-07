#include <task1.h>
#include <syscall.h>
#include <rpsserver.h>
#include <rpsclient.h>
#include <perfmon.h>

void task1() {
	Create(1, rps_server);
	for (int i = 0; i < 4; i++) Create(1, rps_client);
}
