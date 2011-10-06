#include <rawio.h>
#include <kernel.h>
#include <test.h>
#include <kerneltest.h>
#include <task1.h>

int main(int argc, char *argv[]) {
	raw_init();
	test_run();
	kerneltest_run();
	kernel_init();
	kernel_createtask(MAX_PRIORITY, task1);
	kernel_runloop();
	return 0;
}
