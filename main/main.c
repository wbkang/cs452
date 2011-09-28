#include <rawio.h>
#include <kernel.h>
#include <test.h>
#include <syscall.h>
#include <string.h>

static void task2() {
	bwprintf(COM2, "Hello, I'm task 2.\n");
	char *str = malloc(1000);
	strcpy(str, "this message reflects the fact that this is task 2");
	bwprintf(COM2, "malloc: %x\n", str);
	bwprintf(COM2, "str: \"%s\"\n", str);
}

static void task1() {
	bwprintf(COM2, "Hello, I'm task 1.\n");
	char *str = malloc(1000);
	strcpy(str, "this message assures you that task 1 is now running");
	bwprintf(COM2, "malloc: %x\n", str);
	bwprintf(COM2, "str: \"%s\"\n", str);
	Create(1, task2);
	Create(1, task2);
	Create(1, task2);
	Create(1, task2);
}

int main(int argc, char* argv[]) {
	raw_init();
	test_run();
	TRACE("######## kernel_init ########\n\n");
	kernel_init();
	TRACE("\n######## kernel_init done ########\n");
	kernel_driver(task1);
//	ASM("swi 0");
//	ASM("swi 1");

//	Create(1, NULL);
//	Pass();

	return 0;
}

