#include <rawio.h>
#include <kernel.h>
#include <test.h>
#include <syscall.h>

int main(int argc, char* argv[]) {
	raw_init();

//	test_run();
	bwprintf(COM2, "kernel_init starting\n");
	kernel_init();
	bwprintf(COM2, "kernel_init finished\n");
	ASM("swi 0");
	ASM("swi 1");

//	Create(1, NULL);
	Pass();


	return 0;
}
