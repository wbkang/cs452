#include <kernel.h>
#include <test.h>
#include <task1.h>
#include <kerneltest.h>
#include <constants.h>

#define FAST 1

void kerneltest_run(); // from kerneltest.c

int main(int argc, char *argv[]) {
	// this is just to get bwio working.
	uart_fifo(COM1, OFF);
	uart_speed(COM1, 2400);
	uart_stopbits(COM1, 2);
	uart_databits(COM1, 8);
	uart_parity(COM1, OFF);
	// init COM2
	uart_fifo(COM2, OFF);

#if FAST && (!(__i386))
	__asm (
		"mrc p15, 0, r0, c1, c0, 0\n\t"
		"ldr r1, =0x40001004\n\t"
		"orr r0, r0, r1\n\t"
		"mcr p15, 0, r0, c1, c0, 0\n\t"
		: : : "r0", "r1"
	);
#endif

	test_run();
	kerneltest_run();
	kernel_init();
	kernel_createtask(MIN_PRIORITY, task1);
	return kernel_run();
}
