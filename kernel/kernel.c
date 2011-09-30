#include <kernel.h>
#include <assembly.h>
#include <hardware.h>
#include <util.h>
#include <rawio.h>
#include <task.h>
#include <assembly.h>
#include <memory.h>
#include <syscall.h>
#include <scheduler.h>

static void install_interrupt_handlers() {
	INSTALL_INTERRUPT_HANDLER(SWI_VECTOR, asm_handle_swi);
}

void kernel_init() {
	TRACE("######## kernel_init ########");
	install_interrupt_handlers();
	mem_init();
	td_init();
	scheduler_init();
	TRACE("######## kernel_init done ########");
}

void handle_swi(register_set *reg) {
	int *r0 = &reg->r[0];
	int req_no = *r0;
	int a1 = reg->r[1];
	int a2 = reg->r[2];
	// int *args = (int *) reg->r[3];

	switch (req_no) {
		case SYSCALL_CREATE:
			*r0 = kernel_createtask(a1, (func_t) a2);
			scheduler_move2ready();
			break;
		case SYSCALL_MYTID:
			scheduler_runmenext();
			*r0 = kernel_mytid();
			break;
		case SYSCALL_MYPARENTTID:
			scheduler_runmenext();
			*r0 = kernel_myparenttid();
			break;
		case SYSCALL_PASS:
			scheduler_move2ready();
			break;
		case SYSCALL_EXIT:
			scheduler_killme();
			break;
		case SYSCALL_MALLOC:
			scheduler_runmenext();
			*r0 = (int) umalloc((uint) a1);
			break;
		default:
			ERROR("unknown system call %d (%x)\n", req_no, req_no);
			break;
	}
}

void kernel_runloop() {
	task_descriptor *td;
	register_set *reg;
	while (!scheduler_empty()) {
		td = scheduler_get();
		reg = &td->registers;
		asm_switch_to_usermode(reg);
		handle_swi(reg);
	}
}

int kernel_createtask(int priority, func_t code) {
	task_descriptor *td = td_new();
	td->state = 0;
	td->priority = priority;
	td->parent_id = kernel_mytid();
	td->registers.r[REG_LR] = (int) Exit;
	td->registers.r[REG_PC] = (int) code;
	td->registers.spsr = 0x10;
	allocate_user_memory(td);
	scheduler_ready(td);
	return td->id;
}

int kernel_mytid() {
	volatile task_descriptor *td = scheduler_running();
	return td ? td->id : 0xdeadbeef;
}

int kernel_myparenttid() {
	volatile task_descriptor *td = scheduler_running();
	return td ? td->parent_id : 0xdeadbeef;
}
