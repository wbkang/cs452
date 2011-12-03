#include <syscall.h>
#include <stdio.h>
#include <task/calib_test.h>
#include <task/task1.h>
#include <task/perfmon.h>
#include <task/k3clients.h>
#include <task/a0.h>
#include <server/timeserver.h>
#include <server/ioserver.h>
#include <funcmap.h>

void task1() {
	timeserver_create();
	ioserver_create(COM1, OFF, 2400, 2, 8, OFF);
	ioserver_create(COM2, OFF, 115200, 1, 8, OFF);

	__init_funclist();

	func_t prog;
//	do {
//		Putstr(COM2, "Program name? (q to quit): ", tid_com2);
//		char buf[1024], *p = buf;
//		while ((*p++=Getc(COM2, tid_com2)) != '\r') Putc(COM2, *(p-1), tid_com2);
//		*(p - 1) = '\0';
//		if (strcmp("q", buf) == 0) ExitKernel(0);
//		Putc(COM2, '\r', tid_com2);
//		prog = find_func_addr(buf);
//	} while (prog == NULL);
	prog = a0;
	Create(PRIORITY_APP, prog);
}
