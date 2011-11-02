#include <memory.h>
#include <stack.h>
#include <scheduler.h>
#include <syscall.h>
#include <task.h>

#define KERNEL_HEAP_PAGES 10

static memptr kernel_heap;
static stack *umpages;
static uint user_mem_start;
static uint user_mem_end;

int mem_init() {
	kernel_heap = (memptr) ALIGN((uint) &_USER_MEM_START, STACK_SIZE);
	user_mem_start = (uint) kernel_heap + STACK_SIZE * KERNEL_HEAP_PAGES;
	user_mem_end = (uint) &_USER_MEM_END;
	uint task_list_size = (user_mem_end - user_mem_start) / STACK_SIZE;

	// initialize user memory pages
	umpages = stack_new(task_list_size);
	for (int i = 0; i < task_list_size; i++) {
		stack_push(umpages, (void*) (user_mem_start + STACK_SIZE * i));
	}

	return task_list_size;
}

void mem_reset() {
	kernel_heap = (memptr) ALIGN((uint) &_USER_MEM_START, STACK_SIZE);
	user_mem_start = ~0;
	user_mem_end = ~0;
}

void* kmalloc(uint size) {
	memptr rv = kernel_heap;
	kernel_heap += NEXTHIGHESTWORD(size);
	ASSERT((uint) kernel_heap < user_mem_start,
			"kernel heap overflow, alloc_size: %d, kernel_heap: %x, user_mem_start: %x",
			size, kernel_heap, user_mem_start);
	return rv;
}

void* umalloc(uint size) {
	task_descriptor *td = scheduler_running();
	memptr rv = td->heap;
	memptr new_heap = rv + NEXTHIGHESTWORD(size);
	ASSERT((uint) td->registers.r[REG_SP] > (uint) new_heap,
			"No more memory. heap: %x, newheap:%x, sp: %x, size: %d",
			(uint) rv, (uint) new_heap, (uint) td->registers.r[REG_SP], size);
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
	td->registers.r[REG_SP] = STACK_SIZE + (int) heap_base;
}

void free_user_memory(task_descriptor *td) {
	uint base = user_mem_start + (((uint) td->heap - user_mem_start) & ~(STACK_SIZE - 1));
	stack_push(umpages, (memptr) base);
}
