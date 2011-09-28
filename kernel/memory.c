#include <memory.h>
#include <util.h>
#include <kernel.h>
#include <stack.h>

#include <hardware.h>
#include <rawio.h>

extern uint _KERNEL_MEM_START;

static memptr kernel_heap;

static stack *umpages;

void mem_init() {
	// initialize kernel heap
	kernel_heap = &_KERNEL_MEM_START;
	// initialize user memory pages
	umpages = stack_new(TASK_LIST_SIZE);
	for (int i = TASK_LIST_SIZE - 1; i != -1; i--) {
		stack_push(umpages, kmalloc(STACK_SIZE));
	}
}

void mem_reset() {
	kernel_heap = &_KERNEL_MEM_START;
}

void* kmalloc(uint size) { // requires size in bytes
	memptr rv = kernel_heap;
	kernel_heap += (size + 3) >> 2; // round up to nearest word
	return rv;
}

void* umalloc(uint size) { // requires size in bytes
	volatile task_descriptor *td = kernel_td_current();
	uint rounded_up = (size + 3) >> 2; // round up to nearest word
	memptr rv = td->heap;
	td->heap += rounded_up;
	/*bwprintf(COM2, "sp: %x, heap: %x, size:%d, roundedup:%d\n",
			(uint) td->registers.r[REG_SP], (uint) td->heap, size,
			rounded_up);*/
	ASSERT(((uint)td->registers.r[REG_SP]) > (uint)td->heap,
			"user task ran out of memory");
	return rv;
}

void* allocate_user_memory() {
	return stack_pop(umpages);
}
