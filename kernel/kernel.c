#include <kernel.h>
#include <assembly.h>
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
	int req_no = VMEM(reg->r[REG_PC] - 4) & 0xFFFFFF;
	int *r0 = &reg->r[0];
	int a1 = *r0;
	int a2 = reg->r[1];
	int a3 = reg->r[2];
	int a4 = reg->r[3];

	int msglen, replylen;

	switch (req_no) {
		case SYSCALL_CREATE:
			*r0 = kernel_createtask(a1, (func_t) a2);
			if (*r0 >= 0) {
				scheduler_move2ready();
			} else {
				scheduler_runmenext();
			}
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
		case SYSCALL_SEND:
			msglen = a4 & 0xFFFF;
			replylen = ((uint ) a4) >> 16;
			*r0 = kernel_send(a1, (char *) a2, msglen, (char *) a3, replylen);
			break;
		case SYSCALL_RECIEVE:
			*r0 = kernel_recieve((int *) a1, (char *) a2, a3);
			break;
		case SYSCALL_REPLY:
			*r0 = kernel_reply(a1, (char *) a2, a3);
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
	if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) return -1;
	uint entry = (uint) code;
	// probably not in the text region
	if (entry < (uint) &_TextStart || entry >= (uint) &_TextEnd) return -3;
	task_descriptor *td = td_new();
	if (!td) return -2;
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

int kernel_send(int tid, char *msg, int msglen, char *reply, int replylen) {
	TRACE("tid: %d (%x)", tid, tid);
	TRACE("msg: %s (%x)", msg, msg);
	TRACE("msglen: %d, (%x)", msglen, msglen);
	TRACE("reply: %s (%x)", reply, reply);
	TRACE("replylen: %d, (%x)", replylen, replylen);
	return 0xdeadface;
}

int kernel_recieve(int *tid, char *msg, int msglen) {
	TRACE("tid: %d (%x)", tid);
	TRACE("msg: %s (%x)", msg, msg);
	TRACE("msglen: %d, (%x)", msglen, msglen);
	return 0xdeadface;
}

int kernel_reply(int tid, char *reply, int replylen) {
	TRACE("tid: %d (%x)", tid);
	TRACE("reply: %s (%x)", reply, reply);
	TRACE("replylen: %d, (%x)", replylen, replylen);
	return 0xdeadface;
}
