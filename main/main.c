#include <rawio.h>
#include <kernel.h>
#include <test.h>
#include <syscall.h>

int main(int argc, char* argv[]) {
	raw_init();

	test_run();

	kernel_init();

	Create(1, NULL);
	Pass();


	return 0;
}
