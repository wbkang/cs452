#include <kernel.h>
#include <assembly.h>
#include <hardware.h>
#include <util.h>
#include <rawio.h>
#include <task.h>
#include <assembly.h>
#include <stack.h>
#include <priorityq.h>
#include <memory.h>
#include <syscall.h>

// the size of user memory in bytes (64 KB)
#define STACK_SIZE 65536

volatile static task_descriptor *current_task_descriptor;

volatile task_descriptor* kernel_curtask() {
	return current_task_descriptor;
}

static stack *user_memory;

static priorityq *task_priority_queue;

static void install_interrupt_handlers() {
	INSTALL_INTERRUPT_HANDLER(SWI_VECTOR, asm_handle_swi);
}

static void user_memory_init() {
	user_memory = stack_new(TASK_LIST_SIZE);
	for (int i = TASK_LIST_SIZE - 1; i != -1; i--) {
		stack_push(user_memory, kmalloc(STACK_SIZE));
	}
}

static void task_queue_init() {
	bwprintf(COM2, "Task queue init\n");
	task_priority_queue = priorityq_new(TASK_LIST_SIZE, NUM_PRIORITY);
}

void handle_swi(register_set *reg, int req_no) {
	bwprintf(COM2, "Syscall %d,%d,%x. stack:%x\n", req_no, reg->r[0], reg->r[1],
			reg->r[REG_SP]);
	current_task_descriptor->registers = *reg;

	switch (reg->r[0]) {
		case SYSCALL_MALLOC:
			reg->r[0] = (int) umalloc((uint) reg->r[1]);
			break;
		default:
			bwprintf(COM2, "Unknown syscall\n");
			break;
	}
}

void kernel_driver(void(*func)()) {
	task_descriptor* td = td_new();

	td->entry_point = (memptr) (uint) func;
	td->heap = stack_pop(user_memory);
	td->registers.r[REG_LR] = (uint) Exit;
	td->stack = td->heap + STACK_SIZE;
	td->parent_id = -1;
	td->priority = 0;
	td->state = 0;

	priorityq_push(task_priority_queue, td, td->priority);

	current_task_descriptor = td;
	asm_switch_to_usermode(td->stack, td->entry_point);
	ASSERT(FALSE, "hell breaks loose. returned from the usermode");
}

void kernel_init() {
	current_task_descriptor = NULL;
	install_interrupt_handlers();
	mem_init();
	td_init();
	user_memory_init();
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

