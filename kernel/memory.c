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

static inline uint kernel_heap_boundary() {
	if (MEM_PROTECTION) {
		return SECTION_SIZE;
	} else {
		return STACK_SIZE;
	}
}

int mem_init() {
	kernel_heap = (memptr) ALIGN((uint) &_USER_MEM_START, kernel_heap_boundary());
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
	kernel_heap = (memptr) ALIGN((uint) &_USER_MEM_START, kernel_heap_boundary());
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
			"No more memory. heap: %x, newheap: %x, sp: %x, size: %d",
			(uint) rv, (uint) new_heap, (uint) td->registers.r[REG_SP], size);
	// if (!((uint) td->registers.r[REG_SP] > (uint) new_heap)) {
	// 	print_stack_trace(td->registers.r[REG_FP], TRUE);
	// 	for (;;);
	// }
	td->heap = new_heap;
	return rv;
}

void* qmalloc(uint size) {  // can be called from kernel or user
	int mode = 0xdeadbeef;
	READ_CPSR(mode);
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

static inline void mmu_set_ap(int idx, int ap) {
	int section_base;
	// read section base table register
	__asm volatile ("mrc p15, 0, %[sec], c2, c0, 0\n\t" : [sec] "=r" (section_base));
//	PRINT("marking %x as %d", idx, ap);
	int entry = VMEM(section_base + idx * 4) & ~(0x11 << 10);
	VMEM(section_base + idx * 4) = entry | (ap << 10);
}

void mem_dcache_flush() {
	#if !(__i386)
		for (int seg = 0; seg < 8; seg++) {
			for (int index = 0; index < 64; index++) {
				__asm volatile (
					"mcr p15, 0, %[input], c7, c14, 2\n\t" :: [input] "r" (seg * index)
				);
			}
		}
		__asm volatile ("mcr p15, 0, r0, c7, c5, 0\n\t" ::: "r0");
	#endif
}

void mem_tlb_flush() {
	mem_dcache_flush();
	__asm(
		"mcr p15,0,%[zero],c8,c7,0\n\t"
		: : [zero] "r" (0)
	);
}

void mem_mmu_off() {
	// TODO this seems to crash everything
//	// flush tlb & turn off mmu & cache
//	int offflags = 0x1005;
//	mem_tlb_flush();
//	__asm(
//		"mrc p15, 0, r0, c1, c0, 0\n\t"
//		"bic r0, r0, %[off]\n\t"
//		"mcr p15, 0, r0, c1, c0, 0\n\t"
//		: : [zero] "r" (0), [off] "r" (offflags) : "r0"
//	);
}

void mem_mmu_on() {
	// turn on mmu
	int ctrlregoff = 0xc0000000;
	int ctrlreg = MEM_CACHE ? 0x40001005 : 0x1;
	__asm volatile (
		"mrc p15, 0, r0, c1, c0, 0\n\t"
		"bic r0, r0, %[ctrlregoff]\n\t"
		"orr r0, r0, %[ctrlreg]\n\t"
		"mcr p15, 0, r0, c1, c0, 0\n\t"
		:  : [ctrlreg] "r" (ctrlreg), [ctrlregoff] "r" (ctrlregoff) : "r0");
}

void mem_protect() {
	mem_mmu_off();
	int dom = 1;
	// domain access protection
	__asm volatile ("mcr p15, 0, %[dom], c3, c0, 0\n\t" :  : [dom] "r" (dom));
	int romprotection = 0x200;
	//turn on ROM protection
	__asm volatile (
		"mrc p15, 0, r0, c1, c0, 0\n\t"
		"orr r0, r0, %[ctrlreg]\n\t"
		"mcr p15, 0, r0, c1, c0, 0\n\t"
		:  : [ctrlreg] "r" (romprotection) : "r0");

	int kernel_code_start_idx = 0x218000 / SECTION_SIZE;
	int kernel_code_end_idx = ((uint)&_KERNEL_MEM_START) / SECTION_SIZE;
	int user_mem_end_idx = ((uint)&_USER_MEM_END) / SECTION_SIZE;

	// from 0 to before the TEXT starts.
	// system r/w, user no
	for (int i = 0; i < kernel_code_start_idx; i++) {
		mmu_set_ap(i, 0x1);
	}

	// from where TEXT starts to the end of TEXT segment
	// system r/o, user r/o
	for (int i = kernel_code_start_idx; i <= kernel_code_end_idx; i++) {
		mmu_set_ap(i, 0x0);
	}

	// from the end of TEXT to kernel stack & DATA
	// system r/w, user r/w
	for (int i = kernel_code_end_idx + 1; i < user_mem_end_idx; i++) {
		mmu_set_ap(i, 0x3);
	}

	// mark kernel stack & data
	// system r/w, user r/w
	for (int i = user_mem_end_idx; i < 0x2000000 / SECTION_SIZE; i++) {
		mmu_set_ap(i, 0x3);
	}

	// mark the rest as invalid sections, except for the device registers
	int section_base;
	__asm volatile ("mrc p15, 0, %[sec], c2, c0, 0\n\t" : [sec] "=r" (section_base));
	for (int i = user_mem_end_idx + 1; i < 4096; i++) {
		switch (i) {
			case 0x808:
				break;
			default: {
				int entry = VMEM(section_base + i * 4) & ~(0x11 << 10);
				VMEM(section_base + i * 4) = entry & ~0x11;
				break;
			}
		}
	}

	mem_mmu_on();
	mem_tlb_flush();
}

void mem_unprotect() {
	mem_mmu_off();
	int dom = 3;
	// domain access unprotection
	__asm volatile ("mcr p15, 0, %[dom], c3, c0, 0\n\t" :  : [dom] "r" (dom));

	//turn off ROM protection
	int romprotection = 0x200;
	__asm volatile (
		"mrc p15, 0, r0, c1, c0, 0\n\t"
		"bic r0, r0, %[ctrlreg]\n\t"
		"mcr p15, 0, r0, c1, c0, 0\n\t"
		:  : [ctrlreg] "r" (romprotection) : "r0");

	for (int i = 0; i < 4096; i++) {
		mmu_set_ap(i, 0x3);
	}

	mem_mmu_on();
	mem_tlb_flush();
}
