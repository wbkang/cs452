#include <task1.h>
#include <syscall.h>
#include <timeserver.h>
#include <perfmon.h>
#include <k3clients.h>
#include <ioserver.h>
#include <a0.h>
#include <stdio.h>

void task1() {
	Create(PRIORITY_TIMESERVER, timeserver);
	ioserver_create(COM1, OFF, 2400, 2, 8, OFF);
	int com2svr = ioserver_create(COM2, OFF, 115200, 1, 8, OFF);

	struct {
		uint priority;
		func_t prog;
		char name[256];
	} programs[] = {
			{ 1, a0, "A0" },
			{ 1, perfmon, "Send-Receive-Reply Performance Test (receiver first, 64 bytes)" },
			{ 10, k3main, "Kernel 3 ???" }
	};

	int program_count = sizeof(programs) / sizeof(programs[0]);
	char menubuf[(sizeof(programs[0].name) + 4) * program_count], *b = menubuf;

	for (int i = 0; i < program_count; i++) {
		b += sprintf(b, "%d.\t%s\n", i, programs[i].name);
	}

	while (TRUE) {
		Putstr(COM2, menubuf, com2svr);
		char c = Getc(COM2, com2svr);
		int idx = c - '0';
		if (idx < program_count) {
			Putstr(COM2, "You have selected: ", com2svr);
			Putc(COM2, c, com2svr);
			Putc(COM2, '\n', com2svr);
			Flush(com2svr);
			Create(programs[idx].priority, programs[idx].prog);
			return;
		}
	}
}
