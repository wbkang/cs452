#include <memory.h>
#include <stack.h>
#include <scheduler.h>
#include <syscall.h>
#include <task.h>

extern uint _KERNEL_MEM_START;
static memptr kernel_heap;
static stack *umpages;

void mem_init(uint task_list_size) {
	// initialize kernel heap
	kernel_heap = &_KERNEL_MEM_START;
	// initialize user memory pages
	umpages = stack_new(task_list_size);
	for (int i = 0; i < TASK_LIST_SIZE; i++) {
		stack_push(umpages, (void*) (USER_MEM_START + STACK_SIZE * i));
	}
}

void mem_reset() {
	kernel_heap = &_KERNEL_MEM_START;
}

void* kmalloc(uint size) {
	memptr rv = kernel_heap;
	kernel_heap += NEXTHIGHESTWORD(size);
	ASSERT((int) kernel_heap < USER_MEM_START, "kernel heap overflow");
	return rv;
}

void* umalloc(uint size) {
	task_descriptor *td = scheduler_running();
	memptr rv = td->heap;
	memptr new_heap = rv + NEXTHIGHESTWORD(size);
	ASSERT((uint) td->registers.r[REG_SP] > (uint) new_heap, "No more memory");
	td->heap = new_heap;
	return rv;
}

void* qmalloc(uint size) {  // can be called from kernel or user
	int mode = 0xdeadbeef;
	#ifndef __i386
		__asm(
			"mrs %[mode], cpsr" "\n\t"
			"and %[mode], %[mode], #0x1f" "\n\t"
			: [mode] "=r" (mode)
		);
	#endif
	switch (mode) {
		case 0x10: return malloc(size); // user
		case 0x13: return kmalloc(size); // service
		default: // not handled
			ERROR("unhandled processor mode in qmalloc");
			return NULL;
	}
}

void allocate_user_memory(task_descriptor *td) {
	memptr heap_base = (memptr) stack_pop(umpages);
	td->heap = heap_base;
	td->registers.r[REG_SP] = BYTES2WORDS(STACK_SIZE) + (int) heap_base;
}

void free_user_memory(task_descriptor *td) {
	uint base = USER_MEM_START + ((uint) (td->heap - USER_MEM_START) & ~(STACK_SIZE - 1));
	stack_push(umpages, (memptr) base);
}
