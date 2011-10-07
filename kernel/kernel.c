#include <kernel.h>
#include <assembly.h>
#include <util.h>
#include <rawio.h>
#include <task.h>
#include <assembly.h>
#include <memory.h>
#include <syscall.h>
#include <scheduler.h>
#include <string.h>
#include <nameserver.h>

static int nameserver_tid;

static void install_interrupt_handlers() {
	INSTALL_INTERRUPT_HANDLER(SWI_VECTOR, asm_handle_swi);
}

void kernel_init() {
	// TRACE("######## start ########");
	install_interrupt_handlers();
	mem_init(TASK_LIST_SIZE);
	td_init();
	scheduler_init();
	nameserver_tid = kernel_createtask(MAX_PRIORITY - 1, nameserver);
	// TRACE("######## end ########");
}

void handle_swi(register_set *reg) {
	int req_no = VMEM(reg->r[REG_PC] - 4) & 0xFFFFFF;
	int *r0 = &reg->r[0];
	int a1 = *r0;
	int a2 = reg->r[1];
	int a3 = reg->r[2];
	int a4 = reg->r[3];

	int msglen, replylen, rv;

	switch (req_no) {
		case SYSCALL_CREATE:
			*r0 = kernel_createtask(a1, (func_t) a2);
			if (*r0 < 0) {
				scheduler_runmenext();
			} else {
				scheduler_move2ready();
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
			msglen = SENDER_MSGLEN(a4);
			replylen = SENDER_REPLYLEN(a4);
			*r0 = kernel_send(a1, (void*) a2, msglen, (void*) a3, replylen);
			if (*r0 < 0) scheduler_runmenext();
			break;
		case SYSCALL_RECEIVE:
			rv = kernel_receive((int *) a1, (void*) a2, a3);
			if (rv < 0) {
				*r0 =  rv; // write the error
				scheduler_runmenext();
			}
			break;
		case SYSCALL_REPLY:
			*r0 = kernel_reply(a1, (void*) a2, a3);
			break;
		case SYSCALL_NAMESERVERTID:
			scheduler_runmenext();
			*r0 = nameserver_tid;
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
	task_descriptor *td = scheduler_running();
	return td ? td->id : 0xdeadbeef;
}

int kernel_myparenttid() {
	task_descriptor *td = scheduler_running();
	return td ? td->parent_id : 0xdeadbeef;
}

int transfer_msg(task_descriptor *sender, task_descriptor *receiver) {
	*((int *) receiver->registers.r[0]) = sender->id;
	// set up lengths
	int sender_msglen = SENDER_MSGLEN(sender->registers.r[3]);
	int receiver_msglen = receiver->registers.r[2];
	int len = MIN(sender_msglen, receiver_msglen);
	// set up message buffers
	void* sender_msg = (void*) sender->registers.r[1];
	void* receiver_msg = (void*) receiver->registers.r[1];
	memcpy(receiver_msg, sender_msg, len);
	// make sender wait for reply
	scheduler_wait4reply(sender);
	// make receiver ready to run
	receiver->registers.r[0] = len; // return to receiver
	scheduler_ready(receiver);
	// return copied bytes
	return len;
}

int transfer_reply(task_descriptor *sender, task_descriptor *receiver) {
	// set up lengths
	int sender_replylen = SENDER_REPLYLEN(sender->registers.r[3]);
	int receiver_replylen = receiver->registers.r[2];
	int len = MIN(sender_replylen, receiver_replylen);
	// set up message buffers
	void* sender_reply = (void*) sender->registers.r[2];
	void* receiver_reply = (void*) receiver->registers.r[1];
	memcpy(sender_reply, receiver_reply, len);
	// make sender ready to run
	sender->registers.r[0] = len; // return to sender
	TD_REMOVE(sender);
	scheduler_ready(sender);
	// make receiver ready to run
	scheduler_ready(receiver);
	// return 0 implying no error
	return 0;
}

int kernel_send(int tid, void* msg, int msglen, void* reply, int replylen) {
	if (TD_IMPOSSIBLE(tid)) return -1;
	task_descriptor *receiver = td_find(tid);
	if (!receiver) return -2;
	task_descriptor *sender = scheduler_running();
	if (receiver->state == TD_STATE_WAITING4SEND) {
		TD_REMOVE(receiver);
		return transfer_msg(sender, receiver);
	}
	scheduler_wait4receive(receiver, sender);
	return 0; // will return later
}

int kernel_receive(int *tid, void* msg, int msglen) {
	task_descriptor *receiver = scheduler_running();
	task_descriptor *sender = td_pop(receiver);
	if (sender) return transfer_msg(sender, receiver);
	scheduler_wait4send(receiver);
	return 0; // will return later
}

int kernel_reply(int tid, void* reply, int replylen) {
	if (TD_IMPOSSIBLE(tid)) return -1;
	task_descriptor *sender = td_find(tid);
	if (!sender) return -2;
	if (sender->state != TD_STATE_WAITING4REPLY) return -3;
	task_descriptor *receiver = scheduler_running();
	return transfer_reply(sender, receiver);
}
