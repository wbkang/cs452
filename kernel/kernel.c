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

static task_descriptor *waiting_on_send;
static task_descriptor *waiting_on_reply;

void scheduler_put(task_descriptor *list, task_descriptor *td) {
	if (!list) {
		list = td;
	} else {
		TD_PUSH(list, td);
	}
}

static void install_interrupt_handlers() {
	INSTALL_INTERRUPT_HANDLER(SWI_VECTOR, asm_handle_swi);
}

void kernel_init() {
	TRACE("######## kernel_init ########");
	install_interrupt_handlers();
	mem_init(TASK_LIST_SIZE);
	td_init();
	scheduler_init();
	TRACE("######## kernel_init done ########");
}

#define SENDER_MSGLEN(a4) ((a4) & 0xFFFF)
#define SENDER_REPLYLEN(a4) (((uint ) a4) >> 16)

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
			*r0 = kernel_send(a1, (char *) a2, msglen, (char *) a3, replylen);
			if (*r0 < 0) {
				scheduler_runmenext();
			}
			break;
		case SYSCALL_RECIEVE:
			*r0 = kernel_recieve((int *) a1, (char *) a2, a3);
			if (*r0 < 0) {
				scheduler_runmenext();
			}
			break;
		case SYSCALL_REPLY:
			scheduler_move2ready();
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

int kernel_send(int tid, char *msg, int msglen, char *reply, int replylen) {
	TRACE("tid: %d (%x)", tid, tid);
	TRACE("msg: %s (%x)", msg, msg);
	TRACE("msglen: %d, (%x)", msglen, msglen);

	if (TD_IMPOSSIBLE(tid)) return -1;
	task_descriptor *reciever = td_find(tid);
	if (!reciever) return -2;
	task_descriptor *sender = scheduler_running();

	if (reciever->state == TD_STATE_WAITING_FOR_SEND) {
		// set up args
		int *reciever_tid = &reciever->registers.r[0];
		char *reciever_msg = (char *) reciever->registers.r[1];
		int reciever_msglen = reciever->registers.r[2];
		int len = MIN(reciever_msglen, msglen);
		// copy the msg to the reciever
		*reciever_tid = tid;
		memcpy(reciever_msg, msg, len);
		// wait for reply
		sender->state = TD_STATE_WAITING_FOR_REPLY;
		scheduler_put(waiting_on_reply, sender);
		// ready reciever
		scheduler_ready(reciever);
		return len;
	} else {
		sender->state = TD_STATE_WAITING_FOR_RECIEVE;
		TD_PUSH(reciever, sender);
		return 0;
	}
}

int kernel_recieve(int *tid, char *msg, int msglen) {
	task_descriptor *reciever = scheduler_running();

	if (TD_EMPTYLIST(reciever)) { // nobody waiting to send
		reciever->state = TD_STATE_WAITING_FOR_SEND;
		scheduler_put(waiting_on_send, reciever);
		return 0;
	} else {
		task_descriptor *sender = td_pop(reciever);
		*tid = sender->id;
		char *sender_msg = (char *) sender->registers.r[1];
		int sender_msglen = SENDER_MSGLEN(sender->registers.r[3]);
		int len = MIN(sender_msglen, msglen);
		memcpy(msg, sender_msg, len);
		// wait for reply
		sender->state = TD_STATE_WAITING_FOR_REPLY;
		scheduler_put(waiting_on_reply, sender);
		// schedule reciever
		scheduler_ready(reciever);
		return len;
	}
}

int kernel_reply(int tid, char *reply, int replylen) {
	TRACE("tid: %d (%x)", tid);
	TRACE("reply: %s (%x)", reply, reply);
	TRACE("replylen: %d, (%x)", replylen, replylen);

	if (TD_IMPOSSIBLE(tid)) return -1;
	task_descriptor *sender = td_find(tid);
	if (!sender) return -2;
	if (sender->state != TD_STATE_WAITING_FOR_REPLY) return -3;

	char *sender_reply = (char *) sender->registers.r[2];
	int sender_replylen = SENDER_REPLYLEN(sender->registers.r[3]);

	memcpy(sender_reply, reply, MIN(sender_replylen, replylen));
	TD_REMOVE(sender); // remove from any list
	scheduler_ready(sender);
	return 0;
}
