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

static int exitkernel, errno;
static int nameserver_tid;
static int idleserver_tid;
static uint old_swi_vector;
static uint old_hwi_vector;
static uint old_dtabort_vector;
static uint old_pfabort_vector;
static uint old_und_vector;

static inline int kernel_mytid();
static inline int kernel_myparenttid();
static inline int kernel_send(int tid);
static inline void kernel_receive();
static inline int kernel_reply(int tid);
static inline int kernel_awaitevent(int irq);
static inline void handle_swi(register_set *reg);
static inline void handle_hwi(int isr, int isr2);
static inline void kernel_irq(int vic, int irq);
void kernel_idleserver() { for (;;); }

static void install_interrupt_handlers() {
	old_swi_vector = READ_INTERRUPT_HANDLER(SWI_VECTOR);
	old_hwi_vector = READ_INTERRUPT_HANDLER(HWI_VECTOR);
	old_pfabort_vector = READ_INTERRUPT_HANDLER(PFABT_VECTOR);
	old_dtabort_vector = READ_INTERRUPT_HANDLER(DTABT_VECTOR);
	old_und_vector = READ_INTERRUPT_HANDLER(UND_VECTOR);
	INSTALL_INTERRUPT_HANDLER(SWI_VECTOR, asm_handle_swi);
	INSTALL_INTERRUPT_HANDLER(HWI_VECTOR, asm_handle_swi);
	INSTALL_INTERRUPT_HANDLER(PFABT_VECTOR, asm_handle_abort);
	INSTALL_INTERRUPT_HANDLER(DTABT_VECTOR, asm_handle_dabort);
	INSTALL_INTERRUPT_HANDLER(UND_VECTOR, asm_handle_abort);
	VMEM(VIC1 + PROTECTION_OFFSET) = 0;
	VMEM(VIC1 + INTSELECT_OFFSET) = 0;
	VMEM(VIC1 + INTENCLR_OFFSET) = ~0;
	mem_dcache_flush();
}

static void uninstall_interrupt_handlers() {
	INSTALL_INTERRUPT_HANDLER(SWI_VECTOR, old_swi_vector);
	INSTALL_INTERRUPT_HANDLER(HWI_VECTOR, old_hwi_vector);
	INSTALL_INTERRUPT_HANDLER(PFABT_VECTOR, old_pfabort_vector);
	INSTALL_INTERRUPT_HANDLER(DTABT_VECTOR, old_dtabort_vector);
	INSTALL_INTERRUPT_HANDLER(UND_VECTOR, old_und_vector);
	VMEM(VIC1 + INTENCLR_OFFSET) = ~0;
	VMEM(VIC2 + INTENCLR_OFFSET) = ~0;
	VMEM(TIMER1_BASE + CRTL_OFFSET) &= ~ENABLE_MASK;
	VMEM(TIMER2_BASE + CRTL_OFFSET) &= ~ENABLE_MASK;
	VMEM(TIMER3_BASE + CRTL_OFFSET) &= ~ENABLE_MASK;
	uptime_teardown();
	VMEM(UART_BASE(COM1) + UART_CTLR_OFFSET) &= ~(TIEN_MASK | RIEN_MASK | MSIEN_MASK);
	VMEM(UART_BASE(COM2) + UART_CTLR_OFFSET) &= ~(TIEN_MASK | RIEN_MASK | MSIEN_MASK);
	mem_dcache_flush();
}

static int kernel_ever_aborted;

void handle_abort(int fp, int dabort) {
	if (kernel_ever_aborted) {
		bwprintf(1, "double fault!\n");
		for(;;);
	}
	kernel_ever_aborted = 1;
	scheduler_running()->registers.r[REG_FP] = fp;
	bwprintf(1, "Current tid: %d. fp:%x\n", scheduler_running()->id, fp);

	volatile int lr; READ_REGISTER(lr);
	bwprintf(1, "possible undefined instruction %x at %x (this is the banked lr)\n", VMEM(lr), lr);

	volatile unsigned int faulttype;
	__asm volatile ("mrc p15, 0, %[ft], c5, c0, 0\n\t" : [ft] "=r" (faulttype));
	faulttype &= 0xf;

	volatile unsigned int faultdomain;
	__asm volatile ("mrc p15, 0, %[fd], c5, c0, 0\n\t" : [fd] "=r" (faultdomain));
	faultdomain >>= 4;
	faultdomain &= 0xf;

	volatile unsigned int datafaultaddr;
	__asm volatile ("mrc p15, 0, %[dfa], c6, c0, 0\n\t" : [dfa] "=r" (datafaultaddr));

	char *faultname = "wtf";

	if ((faulttype >> 2) == 0) {
		faultname = "alignment";
	} else if (faulttype == 5) {
		faultname = "translation";
	} else if (faulttype == 9) {
		faultname = "domain";
	} else if (faulttype == 13) {
		faultname = "permission";
	} else if (faulttype == 8) {
		faultname = "external abort on noncacheable";
	}

	bwprintf(1, "possible faulttype: %s (%d), faultdomain: %d, datafaultaddr: %x\n",
			faultname, faulttype, faultdomain, datafaultaddr);

	volatile int cpsr;
	READ_CPSR(cpsr);
	cpsr &= 0x1f;

	char *cpsrstr;
	switch(cpsr) {
		case 19: cpsrstr = "svc mode!?"; break;
		case 23:
			if (dabort) cpsrstr = "data abort mode caused by lr - 8";
			else cpsrstr = "prefetch abort mode caused by lr - 4"; break;
		case 27: cpsrstr = "undefined instruction"; break;
		default: cpsrstr = "wtf"; break;
	}

	bwprintf(1, "cpsr: %d (%x) interpretation: %s\n", cpsr, cpsr, cpsrstr);

	td_print_crash_dump();
	for (;;);
}

void kernel_init() {
	mem_mmu_on();
	if (MEM_PROTECTION) {
		mem_protect();
	}
	install_interrupt_handlers();
	int task_list_size = mem_init();
	td_init(task_list_size);
	scheduler_init();
	exitkernel = FALSE;
	nameserver_tid = kernel_createtask(PRIORITY_NAMESERVER, nameserver);
	idleserver_tid = kernel_createtask(PRIORITY_IDLESERVER, kernel_idleserver);
	mem_dcache_flush();
}

static inline void handle_swi(register_set *reg) {
	int req_no = VMEM(reg->r[REG_PC] - 4) & 0x00ffffff;
	int *r0 = reg->r;
	int a1 = *r0;
	int a2 = reg->r[1]; // int a3 = reg->r[2]; int a4 = reg->r[3];
	switch (req_no) {
		case SYSCALL_CREATE:
			*r0 = kernel_createtask(a1, (func_t) a2);
			if (*r0 < 0) scheduler_runmenext(); else scheduler_readyme();
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
			scheduler_readyme();
			break;
		case SYSCALL_EXIT:
			scheduler_freeme();
			break;
		case SYSCALL_MALLOC:
			scheduler_runmenext();
			*r0 = (int) umalloc((uint) a1);
			break;
		case SYSCALL_SEND:
			*r0 = kernel_send(a1);
			if (*r0 < 0) scheduler_runmenext();
			break;
		case SYSCALL_RECEIVE:
			kernel_receive();
			break;
		case SYSCALL_REPLY:
			*r0 = kernel_reply(a1);
			break;
		case SYSCALL_NAMESERVERTID:
			scheduler_runmenext();
			*r0 = nameserver_tid;
			break;
		case SYSCALL_AWAITEVENT:
			*r0 = kernel_awaitevent(a1);
			if (*r0 < 0) scheduler_runmenext();
			break;
		case SYSCALL_EXITKERNEL:
			errno = a1;
			exitkernel = TRUE;
			break;
		case SYSCALL_CRASHDUMP:
			td_print_crash_dump();
			exitkernel = TRUE;
			break;
		default:
			ERROR("unknown system call %d (%x)\n", req_no, req_no);
			break;
	}
}

static inline void handle_hwi(int isr, int isr2) {
	if (isr & INT_MASK(TC1UI)) kernel_irq(VIC1, TC1UI);
	if (isr & INT_MASK(UART1RXINTR1)) kernel_irq(VIC1, UART1RXINTR1);
	if (isr & INT_MASK(UART2RXINTR1)) kernel_irq(VIC1, UART2RXINTR1);
	if (isr2 & INT_MASK(INT_UART1)) kernel_irq(VIC2, INT_UART1);
	if (isr2 & INT_MASK(INT_UART2)) kernel_irq(VIC2, INT_UART2);
	scheduler_readyme();
}

static inline void kernel_irq(int vic, int irq) {
	scheduler_triggerevent(irq);
	VMEM(vic + INTENCLR_OFFSET) = INT_MASK(irq);
}

int kernel_run() {
	uptime_reset();
	uint kernel_start = uptime();
	uint time_idle_start = 0;
	uint idletime = 0;
	kernel_ever_aborted = 0;
	while (LIKELY(!exitkernel)) {
		ASSERT(!scheduler_empty(), "no task to schedule");
		task_descriptor *td = scheduler_get();
		if (td->id == idleserver_tid) time_idle_start = uptime();
		register_set *reg = &td->registers;
		if (MEM_PROTECTION) mem_tlb_flush();
		int cpsr = asm_switch_to_usermode(reg);
		kernel_check_memory(td);
		if ((cpsr & 0x1f) == 0x12) {
			reg->r[REG_PC] -= 4;
			int vic1 = VMEM(VIC1 + IRQSTATUS_OFFSET);
			int vic2 = VMEM(VIC2 + IRQSTATUS_OFFSET);
			handle_hwi(vic1, vic2);
		} else {
			handle_swi(reg);
		}
		if (td->id == idleserver_tid) idletime += uptime() - time_idle_start;
	}
	int up = uptime() - kernel_start;
	int percent10 = ((long long) idletime * 10 * 100 + 5) / (long long) up;
	bwprintf(1, "uptime: %d, idle: %d (%d.%d%%)", up, idletime, percent10 / 10, percent10 % 10);
	uninstall_interrupt_handlers();
	if (MEM_PROTECTION) {
		mem_unprotect();
	}
	return errno;
}

inline int kernel_createtask(int priority, func_t code) {
	if (UNLIKELY(priority < 0 || priority > MAX_PRIORITY)) return -1;
	uint entry = (uint) code;
	if (UNLIKELY(entry < (uint) &_TextStart || entry >= (uint) &_TextEnd)) return -3;
	task_descriptor *td = td_new();
	if (UNLIKELY(!td)) return -2;
	td->parent_id = kernel_mytid();
	td->priority = priority;
	td->registers.spsr = 0x50;
	td->registers.r[REG_LR] = (int) Exit;
	td->registers.r[REG_PC] = entry;
	allocate_user_memory(td);
	scheduler_ready(td);
	return td->id;
}

static inline int kernel_mytid() {
	task_descriptor *td = scheduler_running();
	return td ? td->id : -1;
}

static inline int kernel_myparenttid() {
	task_descriptor *td = scheduler_running();
	return td ? td->parent_id : -1;
}

static inline void transfer_msg(task_descriptor *sender, task_descriptor *receiver) {
	ASSERT(sender->id != receiver->id, "wtf srr tx to itself tid: %d", sender->id);
	int *sender_r = sender->registers.r;
	int *receiver_r = receiver->registers.r;
	// put sender id into the int pointed to by a1 of Receive()
	*((int*) receiver_r[0]) = sender->id;
	// set up lengths
	int sender_msglen = SENDER_MSGLEN(sender_r[3]);
	int receiver_msglen = receiver_r[2];
	ASSERT(receiver_msglen >= sender_msglen, "sending msg too big sid:%d, size:%d, rid:%d, size:%d",
			sender->id, sender_msglen, receiver->id, receiver_msglen);
	int len = min(sender_msglen, receiver_msglen);
	// set up message buffers
	void* sender_msg = (void*) sender_r[1];
	void* receiver_msg = (void*) receiver_r[1];
	memcpy(receiver_msg, sender_msg, len);
	// make sender wait for reply
	scheduler_wait4reply(sender, receiver->id);
	// make receiver ready to run
	receiver_r[0] = len; // receiver rv
	scheduler_ready(receiver);
}

static inline void transfer_reply(task_descriptor *sender, task_descriptor *receiver) {
	int *sender_r = sender->registers.r;
	int *receiver_r = receiver->registers.r;
	// set up lengths
	int sender_replylen = SENDER_REPLYLEN(sender_r[3]);
	int receiver_replylen = receiver_r[2];
	ASSERT(sender_replylen >= receiver_replylen, "reply msg too big. sid: %d, srl:%d, rid:%d, rrl:%d",
			sender->id, sender_replylen, receiver->id, receiver_replylen);
	int len = min(sender_replylen, receiver_replylen);
	// set up message buffers
	void* sender_reply = (void*) sender_r[2];
	void* receiver_reply = (void*) receiver_r[1];
	memcpy(sender_reply, receiver_reply, len);
	// make sender ready to run
	sender_r[0] = len; // sender rv
	scheduler_ready(sender);
	// make receiver ready to run
	scheduler_ready(receiver);
}

static inline int kernel_send(int tid) {
	if (UNLIKELY(td_impossible(tid))) return -1;
	task_descriptor *receiver = td_find(tid);
	if (UNLIKELY(!receiver)) return -2;
	task_descriptor *sender = scheduler_running();
	ASSERT(receiver != sender, "you can't send a message to yourself: %d", receiver->id);
	if (receiver->state == TD_STATE_WAITING4SEND) {
		transfer_msg(sender, receiver);
	} else {
		scheduler_wait4receive(receiver, sender);
	}
	return 0;
}

static inline void kernel_receive() {
	task_descriptor *receiver = scheduler_running();
	if (td_has_children(receiver)) {
		task_descriptor *child = receiver->_head_child;
		while(child) {
			ASSERT(child->id != receiver->id, "receiver %d a child of itself.");
			child = child->_next;
		}
		transfer_msg(td_shift_child(receiver), receiver);
	} else {
		scheduler_wait4send(receiver);
	}
}

static inline int kernel_reply(int tid) {
	if (UNLIKELY(td_impossible(tid))) return -1;
	task_descriptor *sender = td_find(tid);
	if (UNLIKELY(!sender)) return -2;
	if (UNLIKELY(sender->state != TD_STATE_WAITING4REPLY)) return -3;
	transfer_reply(sender, scheduler_running());
	return 0;
}

static inline int kernel_awaitevent(int eventid) {
	int irq, vic;
	switch (eventid) {
		case EVENT_TIMER1:
			vic = VIC1;
			irq = TC1UI;
			break;
		case EVENT_UART1RX:
			vic = VIC1;
			irq = UART1RXINTR1;
			break;
		case EVENT_UART2RX:
			vic = VIC1;
			irq = UART2RXINTR1;
			break;
		case EVENT_UART1:
			vic = VIC2;
			irq = INT_UART1;
			break;
		case EVENT_UART2:
			vic = VIC2;
			irq = INT_UART2;
			break;
		default: // unreachable
			ASSERT(0, "bad event id: %d", eventid);
			vic = -1;
			irq = -1;
			break;
	}
	int irqmask = INT_MASK(irq);
	if (VMEM(vic + RAWINTR_OFFSET) & irqmask) {
		ASSERT((VMEM(vic + INTENABLE_OFFSET) & irqmask) == 0, "irq %d is on", irq);
		scheduler_readyme();
	} else {
		VMEM(vic + INTENABLE_OFFSET) = irqmask;
		scheduler_bindevent(irq);
	}
	return 0;
}

void kernel_check_memory(task_descriptor *td) {
	int sp = td->registers.r[REG_SP];
	int fp = td->registers.r[REG_FP];
	// int lr = td->registers.r[REG_LR];
	// int pc = td->registers.r[REG_PC];

	ASSERT(
		sp > (uint) td->heap,
		"Task %d stack overflow. heap: %x, sp: %x",
		td->id, (uint) td->heap, (uint) td->registers.r[REG_SP]
	);
	ASSERT(fp < 0x2000000, "task %d fp out of range %x", fp);
}

/*
 * Syscall API
 */

int Send(int tid, void* msg, int msglen, void* reply, int replylen) {
	if (UNLIKELY(((uint) msglen | (uint) replylen) & 0xffff0000)) {
		ASSERT(0, "msglen: %d, replylen: %d TOOBIG", msglen, replylen);
		return -3; // unreachable
	}
	return asm_Send(tid, msg, reply, (replylen << 16) | msglen);
}

int ReplyInt(int tid, int rv) {
	return Reply(tid, &rv, sizeof rv);
}

