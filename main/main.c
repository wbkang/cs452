#include <rawio.h>
#include <kernel.h>
#include <test.h>
#include <kerneltest.h>
#include <task1.h>

#define FAST 1

int main(int argc, char *argv[]) {
	raw_init();


#if FAST
		__asm (
				"mrc p15, 0, r0, c1, c0, 0\n\t"
				"mov r1, r0\n\t"
				"mov r0, #1\n\t"
				"bl bwputr\n\t"
				"mrc p15, 0, r0, c1, c0, 0\n\t"
				"ldr r1, =0x40000000\n\t"
				"orr r0, r0, r1\n\t"
				"mcr p15, 0, r0, c1, c0, 0\n\t" : : : "r0", "r1");
#endif
	test_run();
	kerneltest_run();
	kernel_init();
	kernel_createtask(MIN_PRIORITY, task1);
	kernel_runloop();
	return 0;
}
