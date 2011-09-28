#include <memory.h>
#include <util.h>
#include <hardware.h>
#include <kernel.h>

#include <rawio.h>

extern uint _KERNEL_MEM_START;
static memptr kernel_mem_start;

void mem_reset() {
	kernel_mem_start = &_KERNEL_MEM_START;
}

void* kmalloc(uint size) {
	volatile task_descriptor *td = kernel_curtask();
	uint rounded_up = ((size + 3) & ~3) >> 2;

	if (td) {
		memptr rv = td->heap;
		td->heap += rounded_up;
		bwprintf(COM2, "sp: %x, heap: %x, size:%d, roundedup:%d\n", (uint)td->registers.r[REG_SP], (uint)td->heap,size, rounded_up);
		ASSERT(((uint)td->registers.r[REG_SP]) > (uint)td->heap, "user task ran out of memory");
		return rv;
	} else {
		kernel_mem_start += rounded_up;
		return kernel_mem_start;
	}
}

