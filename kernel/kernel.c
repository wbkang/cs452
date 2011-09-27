#include <kernel.h>
#include <assembly.h>
#include <hardware.h>
#include <util.h>
#include <rawio.h>
#include <task.h>
#include <assembly.h>
#include <stack.h>
#include <priorityq.h>

// the size of a user task stack in words. 64k
#define STACK_SIZE 16384

volatile static task_descriptor *current_task_descriptor;

static stack *stack_storage;

static priorityq *task_priority_queue;

static void install_interrupt_handlers() {
	INSTALL_INTERRUPT_HANDLER(SWI_VECTOR, asm_handle_swi);
}

static void stack_storage_init() {
	memptr cur_mem_top = mem_top();
	cur_mem_top = (memptr) ROUND_UP(cur_mem_top, 4096);

	bwprintf(COM2, "Using %x as the usermode stacks storage.\n", cur_mem_top);

	stack_storage = new_stack(TASK_LIST_SIZE, &cur_mem_top);

	for (int i = TASK_LIST_SIZE - 1; i != -1; i--) {
		stack_push(stack_storage, cur_mem_top);
		cur_mem_top += STACK_SIZE;
	}

	mem_mark_occupied(cur_mem_top);
}

static void task_queue_init() {
	bwprintf(COM2, "Task queue init\n");
	memptr heap = mem_top();
	task_priority_queue = priorityq_init(TASK_LIST_SIZE, NUM_PRIORITY, &heap);
	// mem_mark_occupied(cur_mem_top);
}

void handle_swi(register_set *reg, int req_no) {
	bwprintf(COM2, "Syscall number: %d\n", req_no);
	bwprintf(COM2, "Syscall sub-number: %d\n", reg->registers[0]);
	bwprintf(COM2, "Syscall args ptr: %x\n", reg->registers[1]);
}

void kernel_runloop() {

}

void kernel_init() {
	current_task_descriptor = NULL;
	install_interrupt_handlers();
	mem_init();
	td_init();
	stack_storage_init();
	task_queue_init();
}

int kernel_createtask(int priority, void(*code)()) {
	// test weather *code is valid

	// generate task id
	//task_descriptor * td = td_new();

	// initialize user stack

	// append task to scheduler ready queue
	// return my task id
	return 0;
}

int kernel_mytid() {
	// return my task id
	return 0;
}

int kernel_myparenttid() {
	// return my parents task id
	return 0;
}

void kernel_passtask() {
	// put me on the ready queue
}

void kernel_exittask() {
	// remove me from all queues and die
}
