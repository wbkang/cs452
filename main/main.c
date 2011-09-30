#include <rawio.h>
#include <kernel.h>
#include <test.h>
#include <syscall.h>
#include <string.h>
#include <kerneltest.h>

static void task2() {
	bwprintf(COM2, "task id: %d, parent's task id: %d\n", MyTid(),
			MyParentsTid());
	Pass();
	bwprintf(COM2, "task id: %d, parent's task id: %d\n", MyTid(),
			MyParentsTid());
	TRACE("Exiting %d", MyTid());
}

static void task1() {
	for (int i = 0; i < 4; i++) {
		int priority = 2 * (i >> 1);
		bwprintf(COM2, "Created: %d\n", Create(priority, task2));
	}
	bwprintf(COM2, "First: exiting\n");
}

int main(int argc, char *argv[]) {
	raw_init();
	test_run();
	kerneltest_run();
	kernel_init();
	kernel_createtask(1, task1);
	kernel_runloop();
	return 0;
}
