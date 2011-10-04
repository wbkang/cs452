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

static int name_server_id;

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
			*r0 = kernel_send(a1, (void*) a2, msglen, (void*) a3, replylen);
			if (*r0 < 0) {
				scheduler_runmenext();
			}
			break;
		case SYSCALL_RECIEVE:
			*r0 = kernel_recieve((int *) a1, (void*) a2, a3);
			if (*r0 < 0) {
				scheduler_runmenext();
			}
			break;
		case SYSCALL_REPLY:
			*r0 = kernel_reply(a1, (char *) a2, a3);
			break;
		case SYSCALL_REGISTERAS:
			*r0 = kernel_registeras((char *) a1);
			break;
		case SYSCALL_WHOIS:
			*r0 = kernel_whois((char *) a1);
			break;
		case SYSCALL_RETURN:
			*r0 = kernel_return(a1, a2);
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

int transfer_msg(task_descriptor *sender, task_descriptor *reciever) {
	*((int *) reciever->registers.r[0]) = sender->id;
	// set up lengths
	int sender_msglen = SENDER_MSGLEN(sender->registers.r[3]);
	int reciever_msglen = reciever->registers.r[2];
	int len = MIN(sender_msglen, reciever_msglen);
	// set up message buffers
	void* sender_msg = (void*) sender->registers.r[1];
	void* reciever_msg = (void*) reciever->registers.r[1];
	memcpy(reciever_msg, sender_msg, len);
	// make sender wait for reply
	scheduler_wait4reply(sender);
	// make reciever ready to run
	reciever->registers.r[0] = len; // return to reciever
	scheduler_ready(reciever);
	// return copied bytes
	return len;
}

int transfer_reply(task_descriptor *sender, task_descriptor *reciever) {
	// set up lengths
	int sender_replylen = SENDER_REPLYLEN(sender->registers.r[3]);
	int reciever_replylen = reciever->registers.r[2];
	int len = MIN(sender_replylen, reciever_replylen);
	// set up message buffers
	void* sender_reply = (void*) sender->registers.r[2];
	void* reciever_reply = (void*) reciever->registers.r[1];
	memcpy(sender_reply, reciever_reply, len);
	// make sender ready to run
	sender->registers.r[0] = len; // return to sender
	scheduler_ready(sender);
	// make reciever ready to run
	scheduler_ready(reciever);
	// return 0 implying no error
	return 0;
}

int kernel_send(int tid, void* msg, int msglen, void* reply, int replylen) {
	TRACE("tid: %d (%x)", tid, tid);
	TRACE("msg: %s (%x)", msg, msg);
	TRACE("msglen: %d, (%x)", msglen, msglen);

	if (TD_IMPOSSIBLE(tid)) return -1;
	task_descriptor *reciever = td_find(tid);
	if (!reciever) return -2;
	task_descriptor *sender = scheduler_running();
	if (reciever->state == TD_STATE_WAITING4SEND) return transfer_msg(sender, reciever);
	scheduler_wait4recieve(reciever, sender);
	return 0; // will return later
}

int kernel_recieve(int *tid, void* msg, int msglen) {
	task_descriptor *reciever = scheduler_running();
	task_descriptor *sender = td_pop(reciever);
	if (sender) return transfer_msg(sender, reciever);
	scheduler_wait4send(reciever);
	return 0; // will return later
}

int kernel_reply(int tid, void* reply, int replylen) {
	TRACE("tid: %d (%x)", tid);
	TRACE("reply: %s (%x)", reply, reply);
	TRACE("replylen: %d, (%x)", replylen, replylen);

	if (TD_IMPOSSIBLE(tid)) return -1;
	task_descriptor *sender = td_find(tid);
	if (!sender) return -2;
	if (sender->state != TD_STATE_WAITING4REPLY) return -3;
	task_descriptor *reciever = scheduler_running();
	return transfer_reply(sender, reciever);
}

int kernel_registeras(char *name) { // mimic send
	int tid = name_server->id;
	void* msg = (void*) name;
	int msglen = strlen(name);
	void* reply = NULL;
	int replylen = 0;
	return kernel_send(tid, msg, msglen, reply, replylen);
}

int kernel_whois(char *name) {
	return 0;
}

int kernel_return(int tid, int rv) { // mimic recieve
	return 0;
}
