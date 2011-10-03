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
	mem_init();
}

void* kmalloc(uint size) {
	memptr rv = kernel_heap;
	kernel_heap += NEXTHIGHESTWORD(size);
	ASSERT((int) kernel_heap < USER_MEM_START, "kernel heap overflow");
	return rv;
}

void* umalloc(uint size) {
	volatile task_descriptor *td = scheduler_running();
	memptr rv = td->heap;
	memptr new_heap = rv + NEXTHIGHESTWORD(size);
	if (((uint) td->registers.r[REG_SP]) > (uint) new_heap) {
		return NULL;
	} else {
		td->heap = new_heap;
		return rv;
	}
}

void* qmalloc(uint size) {
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
	// add stack size to get stack pointer address.
	// this will technically point to the next tasks heap,
	// but since its a full stack it will increment before pushing
	td->registers.r[REG_SP] = ((int) td->heap) + BYTES2WORDS(STACK_SIZE);
}

void free_user_memory(task_descriptor *td) {
	stack_push(umpages, td->heap_base);
}
