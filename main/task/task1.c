#include <syscall.h>
#include <stdio.h>
#include <task/calib_test.h>
#include <task/task1.h>
#include <task/perfmon.h>
#include <task/k3clients.h>
#include <task/a0.h>
#include <server/timeserver.h>
#include <server/ioserver.h>

void task1() {
	Create(PRIORITY_TIMESERVER, timeserver);
	ioserver_create(COM1, OFF, 2400, 2, 8, OFF);
	int tid_com2 = ioserver_create(COM2, OFF, 115200, 1, 8, OFF);

	struct {
		uint priority;
		func_t prog;
		char name[256];
	} programs[] = {
		{ 7, a0, "A0" },
		{ 1, perfmon, "Send-Receive-Reply Performance Test (receiver first, 64b)" },
		{ 10, k3main, "Kernel 3 ???" },
		{ 1, calib_test, "Calibration test executable" },
	};

	int program_count = sizeof(programs) / sizeof(programs[0]);
	char menubuf[(sizeof(programs[0].name) + 4) * program_count];
	char *b = menubuf;

	for (int i = 0; i < program_count; i++) {
		b += sprintf(b, "%d.\t%s\n", i, programs[i].name);
	}

	int idx;
	char c;

	do {
		Putstr(COM2, menubuf, tid_com2);
		c = Getc(COM2, tid_com2);
		idx = c - '0';
	} while (!(0 <= idx && idx <= program_count));

	Putstr(COM2, "You have selected: ", tid_com2);
	Putc(COM2, c, tid_com2);
	Putc(COM2, '\n', tid_com2);
	Flush(tid_com2);
	Create(programs[idx].priority, programs[idx].prog);
}
