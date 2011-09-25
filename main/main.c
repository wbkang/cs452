#include <rawio.h>
#include <kernel.h>
#include <test.h>

int main(int argc, char* argv[]) {
	raw_init();

	test_run();

	kernel_init();

	return 0;
}
