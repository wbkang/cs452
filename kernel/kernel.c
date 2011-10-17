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
static task_descriptor *eventblocked[NUM_IRQS];
static uint old_swi_vector;
static uint old_hwi_vector;

static inline int kernel_mytid();
static inline int kernel_myparenttid();
static inline void kernel_exit();
static inline int kernel_send(int tid);
static inline void kernel_receive();
static inline int kernel_reply(int tid);
static inline int kernel_awaitevent(int irq);
static inline void handle_swi(register_set *reg);
static inline void handle_hwi(int isr);
static inline void kernel_irq(int irq);
static inline void kernel_idleserver() { for (;;); }

static void flush_dcache() {
	for (int seg = 0; seg < 8; seg++) {
		for (int index = 0; index < 64; index++) {
			__asm volatile (
				"mcr p15, 0, %[input], c7, c14, 2\n\t" :: [input] "r" (seg * index)
			);
		}
	}
	__asm volatile ("mcr p15, 0, r0, c7, c5, 0\n\t" ::: "r0");
}

static void install_interrupt_handlers() {
	old_swi_vector = READ_INTERRUPT_HANDLER(SWI_VECTOR);
	old_hwi_vector = READ_INTERRUPT_HANDLER(HWI_VECTOR);
	INSTALL_INTERRUPT_HANDLER(SWI_VECTOR, asm_handle_swi);
	INSTALL_INTERRUPT_HANDLER(HWI_VECTOR, asm_handle_swi);
	VMEM(VIC1 + PROTECTION_OFFSET) = 0;
	VMEM(VIC1 + INTSELECT_OFFSET) = 0;
	VMEM(VIC1 + INTENCLR_OFFSET) = ~0;
	flush_dcache();
}

static void uninstall_interrupt_handlers() {
	INSTALL_INTERRUPT_HANDLER(SWI_VECTOR, old_swi_vector);
	INSTALL_INTERRUPT_HANDLER(HWI_VECTOR, old_hwi_vector);
	VMEM(VIC1 + INTENCLR_OFFSET) = ~0;
	VMEM(TIMER1_BASE + CRTL_OFFSET) &= ~ENABLE_MASK;
	VMEM(TIMER2_BASE + CRTL_OFFSET) &= ~ENABLE_MASK;
	VMEM(TIMER3_BASE + CRTL_OFFSET) &= ~ENABLE_MASK;
	flush_dcache();
}

void kernel_init() {
	install_interrupt_handlers();
	mem_init(TASK_LIST_SIZE);
	td_init();
	scheduler_init();
	for (int irq = 0; irq < NUM_IRQS; irq++) {
		eventblocked[irq] = NULL;
	}
	exitkernel = FALSE;
	nameserver_tid = kernel_createtask(PRIORITY_NAMESERVER, nameserver);
	idleserver_tid = kernel_createtask(PRIORITY_IDLESERVER, kernel_idleserver);
}

static inline void handle_swi(register_set *reg) {
	int req_no = VMEM(reg->r[REG_PC] - 4) & 0x00ffffff;
	int *r0 = &reg->r[0];
	int a1 = *r0;
	int a2 = reg->r[1]; // int a3 = reg->r[2]; int a4 = reg->r[3];
	switch (req_no) {
		case SYSCALL_CREATE:
			*r0 = kernel_createtask(a1, (func_t) a2);
			if (*r0 < 0) scheduler_runmenext(); else scheduler_move2ready();
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
			kernel_exit();
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
		default:
			ERROR("unknown system call %d (%x)\n", req_no, req_no);
	}
}

static inline void handle_hwi(int isr) {
	if (isr & INT_MASK(TC1UI)) kernel_irq(TC1UI);
	if (isr & INT_MASK(TC2UI)) kernel_irq(TC2UI);
	if (isr & INT_MASK(UART1RXINTR1)) kernel_irq(UART1RXINTR1);
	if (isr & INT_MASK(UART1TXINTR1)) kernel_irq(UART1TXINTR1);
	if (isr & INT_MASK(UART2RXINTR1)) kernel_irq(UART2RXINTR1);
	if (isr & INT_MASK(UART2TXINTR1)) kernel_irq(UART2TXINTR1);
	scheduler_move2ready();
}

static inline void kernel_irq(int irq) {
	task_descriptor *td = eventblocked[irq];
	ASSERT(td, "No task is event blocked on irq #%d", irq);
	VMEM(VIC1 + INTENCLR_OFFSET) = INT_MASK(irq);
	eventblocked[irq] = NULL;
	scheduler_ready(td);
}

static inline uint uptime() {
    return VMEM(0x80810060);
}

int kernel_run() {
	int kernel_start = uptime();
	int time_idle_start = 0;
	int idletime = 0;
	while (LIKELY(!exitkernel)) {
		ASSERT(!scheduler_empty(), "no task to schedule");
		task_descriptor *td = scheduler_get();
		if (td->id == idleserver_tid) time_idle_start = uptime();
		register_set *reg = &td->registers;
		int cpsr = asm_switch_to_usermode(reg);
		if ((cpsr & 0x1f) == 0x12) {
			handle_hwi(VMEM(VIC1 + IRQSTATUS_OFFSET));
			reg->r[REG_PC] -= 4;
		} else {
			handle_swi(reg);
		}
		if (td->id == idleserver_tid) idletime += uptime() - time_idle_start;
	}
	int up = uptime() - kernel_start;
	int percent10 = (10 * 100 * idletime) / up;
	PRINT("uptime: %d, idle: %d (%d.%d%%)", up, idletime, percent10 / 10, percent10 % 10);
	uninstall_interrupt_handlers();
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

static inline void kernel_exit() {
	task_descriptor *receiver = scheduler_running();
	while (!td_list_empty(receiver)) { // clearout receive blocked tasks
		task_descriptor *sender = td_list_pop(receiver);
		sender->registers.r[0] = -2;
		scheduler_ready(sender);
	}
	for (int irq = 0; irq < NUM_IRQS; irq++) { // remove me from eventblocked queue
		if (eventblocked[irq] == receiver) {
			VMEM(VIC1 + INTENCLR_OFFSET) = INT_MASK(irq);
			eventblocked[irq] = NULL;
		}
	}
	free_user_memory(receiver);
	td_free(receiver);
}

static inline void transfer_msg(task_descriptor *sender, task_descriptor *receiver) {
	int *sender_r = sender->registers.r;
	int *receiver_r = receiver->registers.r;
	// put sender id into the int pointed to by a1 of Receive()
	*((int*) receiver_r[0]) = sender->id;
	// set up lengths
	int sender_msglen = SENDER_MSGLEN(sender_r[3]);
	int receiver_msglen = receiver_r[2];
	int len = MIN(sender_msglen, receiver_msglen);
	// set up message buffers
	void* sender_msg = (void*) sender_r[1];
	void* receiver_msg = (void*) receiver_r[1];
	memcpy(receiver_msg, sender_msg, len);
	// make sender wait for reply
	scheduler_wait4reply(sender);
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
	int len = MIN(sender_replylen, receiver_replylen);
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
	if (receiver->state == TD_STATE_WAITING4SEND) {
		transfer_msg(sender, receiver);
	} else {
		scheduler_wait4receive(receiver, sender);
	}
	return 0;
}

static inline void kernel_receive() {
	task_descriptor *receiver = scheduler_running();
	if (td_list_empty(receiver)) {
		scheduler_wait4send(receiver);
	} else {
		transfer_msg(td_list_pop(receiver), receiver);
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

static inline int kernel_awaitevent(int irq) {
	switch (irq) {
		case TC1UI:
		case TC2UI:
		case UART1RXINTR1:
		case UART1TXINTR1:
		case UART2RXINTR1:
		case UART2TXINTR1:
			ASSERT(!eventblocked[irq], "irq %d bound to %d", irq, eventblocked[irq]->id);
			task_descriptor *cur_task = scheduler_running();
			int irqmask = INT_MASK(irq);
			if (VMEM(VIC1 + RAWINTR_OFFSET) & irqmask) {
				scheduler_move2ready();
			}
			else {
				VMEM(VIC1 + INTENABLE_OFFSET) = irqmask;
				eventblocked[irq] = cur_task;
				scheduler_wait4event(cur_task);
			}
			return 0;
		default:
			ERROR("invalid irq (%d)", irq);
			return -1;
	}
}
