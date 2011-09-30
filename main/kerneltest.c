#include <test.h>
#include <util.h>
#include <hardware.h>
#include <memory.h>
#include <heap.h>
#include <stack.h>
#include <rawio.h>
#include <priorityq.h>

void kerneltest_run() {
	mem_reset();
	TRACE("######## kerneltest ########\n");

	TRACE("\n######## kerneltest done ########");
	mem_reset();
}
