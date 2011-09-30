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

volatile task_descriptor *kernel_td_current() {
	return td_current;
}

static priorityq *ready_queue;

static int killme;

static void install_interrupt_handlers() {
	INSTALL_INTERRUPT_HANDLER(SWI_VECTOR, asm_handle_swi);
}

static void task_queue_init() {
	TRACE("\tready queue initialized\n");
	ready_queue = priorityq_new(TASK_LIST_SIZE, NUM_PRIORITY);
}

void handle_swi(register_set *reg) {
	int req_no = reg->r[0];
	void** args = (void**) reg->r[1];
	int *rv = reg->r;

	TRACE( ">handle_swi: req_no: %d\n", req_no);
	// reginfo(reg);

	switch (req_no) {
		case SYSCALL_CREATE:
			*rv = kernel_createtask((int) args[0], (func_t) (uint) args[1]);
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
			TRACE("unknown system call %d (%x)\n", req_no, req_no);
			die();
			break;
	}

	TRACE(">handle_swi done:\n>\ttid: %d\n", td_current->id);
	// reginfo(reg);
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
	for (int i = 0; i < 13; i++) {
		td->registers.r[i] = 0xbeef0000 + i;
	}
	td->registers.r[REG_LR] = (int) Exit;
	td->registers.r[REG_PC] = (int) code;
	td->registers.spsr = 0x10;
	td->heap = (memptr) allocate_user_memory(); // top of allocated memory
	td->heap_base = td->heap;
	td->registers.r[REG_SP] = (int) td->heap + (STACK_SIZE >> 2);

	priorityq_push(ready_queue, td, priority);

	// TRACE(">\tcreatetask tid:%d heap:%x pc:%x\n", td->id, td->heap, td->registers.r[REG_PC]);
	return td->id;
}

int kernel_mytid() {
	return td_current ? td_current->id : -1;
}

int kernel_myparenttid() {
	return td_current->parent_id;
}

void kernel_passtask() {
	TRACE("kernel_passtask()\n");
//	priorityq_push(ready_queue, (task_descriptor *) td_current,
//			td_current->priority);
//	td_current = priorityq_pop(ready_queue); // grab next task
//	priorityq_push(ready_queue, (task_descriptor *) td_current,
//			td_current->priority); // schedule next task
}

void kernel_runloop() {
	register_set *reg;
	while (!PRIORITYQ_EMPTY(ready_queue)) {
		td_current = priorityq_pop(ready_queue);
		TRACE("scheduling task with id %d and priority %d\n", td_current->id, td_current->priority);
		reg = &((task_descriptor *) td_current)->registers;
		asm_switch_to_usermode(reg);
		killme = FALSE;
		handle_swi(reg);
		if (!killme) {
			priorityq_push(ready_queue, (task_descriptor *) td_current,
					td_current->priority);
		}
	}
}

void kernel_exittask() {
	TRACE("kernel_exittask() -- pq size:%d\n", ready_queue->len);
	free_user_memory(td_current->heap_base);
	td_free((task_descriptor *) td_current);
	killme = TRUE;
}
