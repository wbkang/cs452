#include <kernel.h>
#include <assembly.h>
#include <hardware.h>
#include <util.h>
#include <rawio.h>
#include <task.h>
#include <assembly.h>
#include <stack.h>

#define STACK_SIZE 65536

volatile static int current_tid;

static void* _stack_memory[TASK_LIST_SIZE];
static stack stack_storage;

static void install_interrupt_handlers() {
	INSTALL_INTERRUPT_HANDLER(SWI_VECTOR, asm_handle_swi);
}

static void stack_storage_init() {
	uint mem_start = _MY_MEM_START;

	stack_init(&stack_storage, _stack_memory, TASK_LIST_SIZE);

	for (int i = 0; i < TASK_LIST_SIZE; i++) {
		stack_push(&stack_storage, (stack_node)(mem_start + i * STACK_SIZE));
	}
}

void handle_swi(register_set *reg, int req_no) {
	bwprintf(COM2, "Syscall number: %d\n", req_no);
	bwprintf(COM2, "Syscall sub-number: %d\n", reg->registers[0]);
	bwprintf(COM2, "Syscall args ptr: %x\n", reg->registers[1]);
}

void kernel_init() {
	current_tid = -1;
	install_interrupt_handlers();
//	td_init();
	stack_storage_init();
}

int kernel_createtask(int priority, void(*code)()) {
	// test weather *code is valid

	// generate task id
	task_descriptor * td = td_new();

	// initialize user stack
	td->parent_id = 0;
	td->priority = priority;


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
