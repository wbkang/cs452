#include <rawio.h>
#include <kernel.h>
#include <test.h>
#include <syscall.h>
#include <string.h>

static void task1() {
	char* str = malloc(1000);
	strcopy(str, "this is a big ol' test string.");
	bwprintf(COM2, "Hi from usermode, wish you were here:\n\tallocated memory %x\n\tstring: \"%s\"\n", str, str);
}

int main(int argc, char* argv[]) {
	raw_init();
	kernel_init();
	test_run();
	bwprintf(COM2, "######## kernel_init ########\n");
	kernel_init();
	bwprintf(COM2, "######## kernel_init done ########\n");
	kernel_driver(task1);
//	ASM("swi 0");
//	ASM("swi 1");

//	Create(1, NULL);
//	Pass();

	return 0;
}

