#include <kernel.h>
#include <assembly.h>
#include <hardware.h>
#include <util.h>
#include <rawio.h>
#include <task.h>
#include <assembly.h>
#include <priorityq.h>
#include <memory.h>
#include <syscall.h>

volatile static task_descriptor *td_current;

volatile task_descriptor* kernel_td_current() {
	return td_current;
}

static priorityq *ready_queue;

static void install_interrupt_handlers() {
	INSTALL_INTERRUPT_HANDLER(SWI_VECTOR, asm_handle_swi);
}

static void task_queue_init() {
	bwprintf(COM2, "\tready queue initialized\n");
	ready_queue = priorityq_new(TASK_LIST_SIZE, NUM_PRIORITY);
}

void handle_swi(register_set *reg, int req_no) {
	bwprintf(
			COM2,
			">system call:\n>\treq_no: %d\n>\tr0: %x\n>\tr1: %x\n>\tstack: %x\n",
			req_no, reg->r[0], reg->r[1], reg->r[REG_SP]);

	volatile task_descriptor *td_in = td_current;

	td_in->registers = *reg;

	req_no = reg->r[0]; // what kinda crap is this...

	int *rv = reg->r;
	void* *args = (void* *) reg->r[1];

	switch (req_no) {
		case SYSCALL_CREATE:
			*rv = kernel_createtask((int) args[0], (func_t) (uint) args[1]);
			// asm_switch_to_usermode(td_current->stack, td_current->entry_point);
			break;
		case SYSCALL_MYTID:
			*rv = kernel_mytid();
			break;
		case SYSCALL_MYPARENTTID:
			*rv = kernel_myparenttid();
			break;
		case SYSCALL_PASS:
			kernel_passtask();
			break;
		case SYSCALL_EXIT:
			kernel_exittask();
			break;
		case SYSCALL_MALLOC:
			*rv = (int) umalloc((uint) args);
			break;
		default:
			bwprintf(COM2, "unknown system call %d (%x)\n", req_no, req_no);
			die();
			break;
	}

	if (td_in->id != td_current->id) {
		*reg = td_current->registers;
		td_in->registers.r[0] = *rv;
	}

	bwprintf(COM2, ">\ttid:%d, lr:%x, pc:%x, spsr:%x\n", td_current->id,
			reg->r[REG_LR], reg->r[REG_PC], reg->spsr);
}

void kernel_driver(func_t code) {
	task_descriptor* td = td_new();
	td->state = 0;
	td->priority = 0;
	td->parent_id = -1;
	td->registers.r[REG_LR] = (uint) Exit;
	td->entry_point = (memptr) (uint) code;
	td->heap = (memptr) allocate_user_memory(); // top of allocated memory
	td->stack = td->heap + (STACK_SIZE >> 2); // bottom of allocated memory

	priorityq_push(ready_queue, td, td->priority);
	td_current = td;
	asm_switch_to_usermode(td->stack, td->entry_point);

	ASSERT(FALSE, "reached unreachable code...");
}

void kernel_init() {
	td_current = NULL;
	install_interrupt_handlers();
	mem_init();
	td_init();
	task_queue_init();
}

int kernel_createtask(int priority, func_t code) {
	// < somehow test weather *code is valid >

	task_descriptor* td = td_new();
	td->state = 0;
	td->priority = priority;
	td->parent_id = kernel_mytid();
	td->registers.r[REG_LR] = (int) Exit;
	td->registers.r[REG_PC] = (int) code;
	td->registers.spsr = 0x600000d0;
	td->heap = (memptr) allocate_user_memory(); // top of allocated memory
	td->stack = td->heap + (STACK_SIZE >> 2); // bottom of allocated memory
	td->registers.r[REG_SP] = (int) td->stack;

	priorityq_push(ready_queue, td, priority);

	bwprintf(COM2, ">\tcreatetask tid:%d heap:%x\n", td->id, td->heap);
	return td->id;
}

int kernel_mytid() {
	return td_current->id;
}

int kernel_myparenttid() {
	return td_current->parent_id;
}

void kernel_passtask() {
	bwprintf(COM2, "kernel_passtask()\n");
	priorityq_push(ready_queue, (task_descriptor *) td_current,
			td_current->priority);
	td_current = priorityq_pop(ready_queue); // grab next task
	priorityq_push(ready_queue, (task_descriptor *) td_current,
			td_current->priority); // schedule next task
}

void kernel_exittask() {
	bwprintf(COM2, "kernel_exittask()\n");
	td_current = priorityq_pop(ready_queue); // grab next task
	priorityq_push(ready_queue, (task_descriptor *) td_current,
			td_current->priority); // schedule next task
}
