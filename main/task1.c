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
	Create(PRIORITY_TIMESERVER, timeserver);
	ioserver_create(COM1, OFF, 2400, 2, 8, OFF);
	int com2svr = ioserver_create(COM2, OFF, 115200, 1, 8, OFF);

	char *menu =
			"Menu:\n"
			"0. A0\n"
			"1. Send-Receive-Reply (receiver first, 64 bytes)\n"
			"2. Kernel 3\n";

	Putstr(COM2, menu, com2svr);

	char c;
	while (TRUE) {
		char c = Getc(COM2, com2svr);
		switch (c) {
			case '0':
				Create(1, a0);
				break;
			case '1':
				Create(1, perfmon);
				break;
			case '2':
				Create(10, k3main);
				break;
			default:
				continue;
		}
	}
}
