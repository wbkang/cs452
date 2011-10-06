#include <task1.h>
#include <syscall.h>
#include <rpsserver.h>
#include <rpsclient.h>

void task1() {
	Create(1, rps_server);
	for (int i = 0; i < 2; i++) Create(0, rps_client);
}
