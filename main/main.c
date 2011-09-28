#include <rawio.h>
#include <kernel.h>
#include <test.h>
#include <syscall.h>

static void task1() {
	char* str = malloc(1000);
	str[0] = 'a';
	str[1] = '\0';
	bwprintf(COM2, "HELLOOOOO from usermode: allocated memory %x, string: %s\n", str, str);
}

int main(int argc, char* argv[]) {
	raw_init();
	kernel_init();
	test_run();
	bwprintf(COM2, "kernel_init starting\n");
	kernel_init();
	bwprintf(COM2, "kernel_init finished\n");
	kernel_driver(task1);
//	ASM("swi 0");
//	ASM("swi 1");

//	Create(1, NULL);
//	Pass();

	return 0;
}

