#include <ioserver.h>
#include <ts7200.h>
#include <hardware.h>
#include <syscall.h>
#include <notifier.h>
#include <queue.h>
#include <util.h>
#include <string.h>

/*
 * max number of chars per timeserver tick
 * 		COM1: 2400 bps * 10 ms = 3
 * 		COM2: 115200 bps * 10 ms = 144
 */

#define BUFFER_SIZE 256
#define INPUT_BLOCKED_QUEUE_SIZE 400

typedef struct {
	int channel;
	queue *input;
	queue *input_blocked;
	queue *output;
	int tid_notifier_general;
	int tx_empty;
	int cts;
} ioserver_state;

typedef struct {
	int channel;
	int fifo;
	int speed;
	int stopbits;
	int databits;
	int parity;
} ioserver_arg;

typedef struct {
	enum { PUTC, GETC, PUTSTR } no;
	union {
		char c;
		char const *str;
	} data;
} ioserver_req;

static inline void uart_init(ioserver_arg* args);
static inline void handle_getc(ioserver_state *state, int tid);
static inline void handle_putc(ioserver_state *state, int tid, char c);
static inline void handle_putstr(ioserver_state *state, int tid, char const *c);
static inline void handle_general(ioserver_state *state);
static inline char rxchar(ioserver_state *state);
static inline void txchar(ioserver_state *state);

static void ioserver() {
	int tid;
	ioserver_arg args;

	// receive args
	Receive(&tid, &args, sizeof args);
	ReplyNull(tid);

	// register name and init uart
	RegisterAs(args.channel == COM1 ? NAME_IOSERVER_COM1 : NAME_IOSERVER_COM2);
	uart_init(&args);

	// init state
	ioserver_state state;
	state.channel = args.channel;
	state.input = queue_new(BUFFER_SIZE);
	state.input_blocked = queue_new(INPUT_BLOCKED_QUEUE_SIZE);
	state.output = queue_new(BUFFER_SIZE);
	int general_event = args.channel == COM1 ? EVENT_UART1 : EVENT_UART2;
	state.tid_notifier_general = notifier_new(PRIORITY_IONOTIFIER, general_event);
	int uartbase = UART_BASE(args.channel);
	state.cts = (VMEM(uartbase + UART_FLAG_OFFSET) & CTS_MASK) || args.channel == COM2;
	state.tx_empty = VMEM(uartbase + UART_FLAG_OFFSET) & TXFE_MASK;
	// sync with notifier
	Receive(&tid, NULL, 0);
	ASSERT(tid == state.tid_notifier_general, "req during init from tid: %d", tid);
	ReplyNull(tid);

	// serve
	for (;;) {
		ioserver_req req;
		int msglen = Receive(&tid, (void*) &req, sizeof(req));
		if (tid == state.tid_notifier_general) {
			handle_general(&state);
		} else if (msglen == sizeof(req)) {
			switch (req.no) {
				case PUTC:
					handle_putc(&state, tid, req.data.c);
					break;
				case GETC:
					handle_getc(&state, tid);
					break;
				case PUTSTR:
					handle_putstr(&state, tid, req.data.str);
					break;
				default:
					ASSERT(FALSE, "bad reqno: %d", req.no);
					ReplyInt(tid, IOSERVER_ERROR_BADREQNO);
					break;
			}
		} else {
			ASSERT(FALSE, "bad data");
			ReplyInt(tid, IOSERVER_ERROR_BADDATA);
		}
	}
}

static inline void uart_init(ioserver_arg *args) {
	int channel = args->channel;
	CHECK_COM(channel);
	uart_fifo(channel, args->fifo);
	uart_speed(channel, args->speed);
	uart_stopbits(channel, args->stopbits);
	uart_databits(channel, args->databits);
	uart_parity(channel, args->parity);

	int uartbase = UART_BASE(channel);
	// enable tx, rx, and ms interrupts
	VMEM(uartbase + UART_CTLR_OFFSET) |= TIEN_MASK | RIEN_MASK | MSIEN_MASK;
	VMEM(uartbase + UART_INTR_OFFSET) |= 1; // clear ms interrupt
}

static inline void handle_general(ioserver_state *state) {
	int uartbase = UART_BASE(state->channel);
	int uart_isr = VMEM(uartbase + UART_INTR_OFFSET);

	// incoming register full
	if (uart_isr & UARTRXINTR) {
		ASSERT(VMEM(uartbase + UART_FLAG_OFFSET) & RXFF_MASK, "incoming register empty");
		ASSERT(!queue_full(state->input), "input queue full");
		int c = rxchar(state);
		if (queue_empty(state->input_blocked)) {
			queue_push(state->input, (void*) c);
		} else {
			int tid = (int) queue_pop(state->input_blocked);
			ReplyInt(tid, c);
		}
	}

	// outgoing register empty
	if (uart_isr & UARTTXINTR) {
		ASSERT(VMEM(uartbase + UART_FLAG_OFFSET) & TXFE_MASK, "outgoing register full");
		state->tx_empty = TRUE;
		VMEM(uartbase + UART_CTLR_OFFSET) &= ~TIEN_MASK;
	}

	// modem status interrupt (cts toggled)
	if (uart_isr & UARTMSINTR) {
		ASSERT(state->channel != COM2, "got MSI on COM2");
		state->cts = VMEM(uartbase + UART_FLAG_OFFSET) & CTS_MASK;
		VMEM(uartbase + UART_INTR_OFFSET) = 1; // clear ms interrupt in hardware
	}

	// send if we can
	if (state->tx_empty && state->cts && !queue_empty(state->output)) {
		txchar(state);
	}

	// unblock the notifier
	ReplyNull(state->tid_notifier_general);
}

static inline char rxchar(ioserver_state *state) {
	int uartbase = UART_BASE(state->channel);
	return (char) (VMEM(uartbase + UART_DATA_OFFSET) & DATA_MASK);
}

static inline void txchar(ioserver_state *state) {
	ASSERT(!queue_empty(state->output), "output empty");
	int uartbase = UART_BASE(state->channel);
	VMEM(uartbase + UART_DATA_OFFSET) = (uint) queue_pop(state->output); // write char
	VMEM(uartbase + UART_CTLR_OFFSET) |= TIEN_MASK; // enable tx interrupt
	VMEM(uartbase + UART_INTR_OFFSET) = 1; // clear ms interrupt in hardware
	state->cts = (state->channel == COM2); // assume cts off if COM1
	state->tx_empty = VMEM(uartbase + UART_CTLR_OFFSET) & TXFE_MASK;
}

static inline void handle_getc(ioserver_state *state, int tid) {
	if (queue_empty(state->input)) {
		queue_push(state->input_blocked, (void*) tid);
	} else {
		ReplyInt(tid, (int) queue_pop(state->input));
	}
}

static inline void handle_putc(ioserver_state *state, int tid, char c) {
	if (UNLIKELY(queue_full(state->output))) {
		ERROR("queue full, channel: %d, char: %c (%x)", state->channel, c, c);
	}
	queue_push(state->output, (void*)(int) c);
	if (state->tx_empty && state->cts) {
		txchar(state);
	}
	ReplyInt(tid, 0);
}

static inline void handle_putstr(ioserver_state *state, int tid, char const *str) {
	if (UNLIKELY(queue_full(state->output))) {
		ERROR("queue full, channel: %d, string: %s (%x)", state->channel, str, str);
	}

	char const * p = str;

	while (*p) {
		queue_push(state->output, (void*)(int) *p++);
	}

	if (state->tx_empty && state->cts && p != str) {
		txchar(state);
	}

	ReplyInt(tid, 0);
}

int ioserver_create(int channel, int fifo, int speed, int stopbits, int databits, int parity) {
	ioserver_arg args;
	args.channel = channel;
	args.fifo = fifo;
	args.speed = speed;
	args.stopbits = stopbits;
	args.databits = databits;
	args.parity = parity;
	int tid = Create(PRIORITY_IOSERVER, &ioserver);
	if (tid < 0) return tid;
	int rv;
	int len = Send(tid, (char*) &args, sizeof args, (char*) &rv, sizeof rv);
	if (len < 0) return len;
	if (rv < 0) return rv;
	return tid;
}

static inline int ioserver_send(int tid, ioserver_req *req) {
	int rv;
	int len = Send(tid, (char*) req, sizeof *req, (char*) &rv, sizeof rv);
	if (len < 0) return len;
	return rv;
}

int ioserver_getc(int tid) {
	ioserver_req req;
	req.no = GETC;
	return ioserver_send(tid, &req);
}

int ioserver_putc(char c, int tid) {
	ioserver_req req;
	req.no = PUTC;
	req.data.c = c;
	return ioserver_send(tid, &req);
}

int ioserver_putstr(char const *str, int tid) {
	ioserver_req req;
	req.no = PUTSTR;
	req.data.str = str;
	return ioserver_send(tid, &req);
}
