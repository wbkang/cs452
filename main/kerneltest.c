#include <test.h>
#include <util.h>
#include <memory.h>
#include <kernel.h>
#include <syscall.h>

static void sayhi() {
	TRACE("hi, i am task %d\n", MyTid());
}
static void kerneltest_max_tasks() {
	for (int i = 0; i <TASK_LIST_SIZE-1; i++) {
		TRACE("CREATED TASK %d\n", Create(i % NUM_PRIORITY, sayhi));
	}
}

static void kerneltest_runner(int priority, void (*test)()) {
	kernel_init();
	kernel_createtask(priority, test);
	kernel_runloop();
	mem_reset();
}




void kerneltest_run() {
	mem_reset();
	TRACE("######## kerneltest ########\n\n");

	kerneltest_runner(0, kerneltest_max_tasks);

	TRACE("\n######## kerneltest done ########\n");
	mem_reset();
}
