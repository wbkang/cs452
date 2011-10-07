#include <rawio.h>
#include <kernel.h>
#include <test.h>
#include <kerneltest.h>
#include <task1.h>

int main(int argc, char *argv[]) {
	raw_init();
#ifndef __X86
//	__asm volatile (
//			"mrc p15, 0, r0, c1, c0, 0 \n\t"
//			"ldr r1, =0x1005\n\t"
//			"orr r0, r0, r1\n\t"
//			"mcr p15, 0, r0, c1, c0, 0 \n\t"
//			: : : "r0", "r1");
#endif
	TRACE("MAIN START");
	test_run();
	kerneltest_run();
	kernel_init();
	kernel_createtask(MAX_PRIORITY, task1);
	kernel_runloop();
	return 0;
}
