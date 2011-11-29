#include <kernel.h>
#include <test.h>
#include <task/task1.h>
#include <kerneltest.h>
#include <constants.h>

void kerneltest_run(); // from kerneltest.c

int main(int argc, char *argv[]) {
	uart_fifo(COM1, OFF);
	uart_speed(COM1, 2400);
	uart_stopbits(COM1, 2);
	uart_databits(COM1, 8);
	uart_parity(COM1, OFF);
	uart_fifo(COM2, OFF);

	test_run();
	kerneltest_run();
	kernel_init();
	kernel_createtask(MIN_PRIORITY, task1);
	return kernel_run();
}
