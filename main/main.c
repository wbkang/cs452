#include <util.h>
#include <ts7200.h>
#include <rawio.h>
#include <hardware.h>
#include <task.h>
#include <memory.h>






int main(int argc, char* argv[]) {
	raw_init();

	test_run();

	kernel_init();

	return 0;
}
