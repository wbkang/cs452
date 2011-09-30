#include <memory.h>
#include <util.h>
#include <stack.h>
#include <scheduler.h>
#include <syscall.h>

extern uint _KERNEL_MEM_START;

static memptr kernel_heap;
static stack *umpages;

void mem_init() {
	// initialize kernel heap
	kernel_heap = &_KERNEL_MEM_START;
	// initialize user memory pages
	umpages = stack_new(TASK_LIST_SIZE);
	for (int i = TASK_LIST_SIZE - 1; i != -1; i--) {
		stack_push(umpages, (void*) (USER_MEM_START + STACK_SIZE * i));
	}
}

void mem_reset() {
	kernel_heap = &_KERNEL_MEM_START;
}

void* kmalloc(uint size) { // requires size in bytes
	memptr rv = kernel_heap;
	kernel_heap += (size + 3) >> 2; // round up to nearest word
	ASSERT((int) kernel_heap < USER_MEM_START, "kernel heap overflow");
	return rv;
}

void* umalloc(uint size) { // requires size in bytes
	volatile task_descriptor *td = scheduler_running();
	uint rounded_up = (size + 3) >> 2; // round up to nearest word
	memptr rv = td->heap;
	td->heap += rounded_up;
	/*TRACE("sp: %x, heap: %x, size:%d, roundedup:%d",
	 (uint) td->registers.r[REG_SP], (uint) td->heap, size,
	 rounded_up);*/
	ASSERT(((uint)td->registers.r[REG_SP]) > (uint)td->heap,
			"user task ran out of memory");
	return rv;
}

void* qmalloc(uint size) { // requires size in bytes
	int mode = 0xdeadbeef;

#ifndef __i386
	__asm(
			"mrs %[mode], cpsr" "\n\t"
			"and %[mode], %[mode], #0x1f" "\n\t"
			: [mode] "=r" (mode)
	);
#endif

	switch (mode) {
		case 0x10: // user
			return malloc(size);
		case 0x13: // service
			return kmalloc(size);
		default: // not handled
			ERROR("unhandled processor mode in qmalloc");
			return NULL;
	}
}

void allocate_user_memory(task_descriptor *td) {
	td->heap_base = (memptr) stack_pop(umpages);
	td->heap = td->heap_base;
	td->registers.r[REG_SP] = (int) td->heap + (STACK_SIZE >> 2);
}

void free_user_memory(task_descriptor *td) {
	stack_push(umpages, td->heap_base);
}
