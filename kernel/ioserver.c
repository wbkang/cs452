#include <ioserver.h>
#include <ts7200.h>
#include <hardware.h>
#include <syscall.h>
#include <notifier.h>
#include <queue.h>
#include <util.h>
#include <string.h>

#define BUFFER_SIZE (1<<10)
#define INPUT_BLOCKED_QUEUE_SIZE 500

typedef struct {
	enum { OUTPUT_EMPTY, OUTPUT_NOT_EMPTY } state;
	queue *input;
	queue *input_blocked;
	queue *output;
	int tid_notifier_general;
	int tx_empty;
	int cts;
//	int tid_notifier_rx;
//	int tid_notifier_tx;
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
	enum { PUTC, GETC } no;
	char channel;
	char c;
} ioserver_req;

static inline void init(ioserver_arg* args);
static inline void handle_getc(ioserver_state *state, int tid);
static inline void handle_putc(ioserver_state *state, int tid, int channel, char c);
//static inline void handle_rx(ioserver_state *state, int channel);
//static inline void handle_tx(ioserver_state *state, int channel);
static inline void handle_general(ioserver_state *state, int channel);
static inline char rxchar(int channel);
static inline void txchar(ioserver_state *state, int channel);

static void ioserver() {
	int tid;
	ioserver_arg args;
	Receive(&tid, &args, sizeof args);
	Reply(tid, NULL, 0);
	RegisterAs((args.channel == COM1) ? NAME_IOSERVER_COM1 : NAME_IOSERVER_COM2);
	init(&args);

	int general_event;
//	int rx_event, tx_event;

	if (args.channel == COM1) {
		general_event = EVENT_UART1;
//		rx_event = EVENT_UART1RX;
//		tx_event = EVENT_UART1TX;
	} else {
		ASSERT(args.channel == COM2, "should never reach here. channel # %d", args.channel);
		general_event = EVENT_UART2;
//		rx_event = EVENT_UART2RX;
//		tx_event = EVENT_UART1TX;
	}

	ioserver_state state;
	state.state = OUTPUT_EMPTY;
	state.input = queue_new(BUFFER_SIZE);
	state.input_blocked = queue_new(INPUT_BLOCKED_QUEUE_SIZE);
	state.output = queue_new(BUFFER_SIZE);
	state.tid_notifier_general = notifier_new(PRIORITY_IONOTIFIER, general_event);
	int uartbase = UART_BASE(args.channel);
	state.cts = (VMEM(uartbase + UART_FLAG_OFFSET) & CTS_MASK) || args.channel == COM2;
	state.tx_empty = VMEM(uartbase + UART_FLAG_OFFSET) & TXFE_MASK;
//	state.tid_notifier_rx = notifier_new(PRIORITY_IONOTIFIER, rx_event);
//	state.tid_notifier_tx = notifier_new(PRIORITY_IONOTIFIER, tx_event);

	Receive(&tid, NULL, 0);
	ASSERT(tid == state.tid_notifier_general, "unexpected request during init from tid: %d", tid);
	ReplyNull(state.tid_notifier_general);
	for (;;) {
		ioserver_req req;
		int msglen = Receive(&tid, (void*) &req, sizeof(req));
		if (tid == state.tid_notifier_general) {
			handle_general(&state, args.channel);
		} /*else if (tid == state.tid_notifier_tx) {
			handle_tx(&state, args.channel);
		} */
		else if (msglen == sizeof(req)) {
			switch (req.no) {
				case PUTC:
					handle_putc(&state, tid, args.channel, req.c);
					break;
				case GETC:
					handle_getc(&state, tid);
					break;
				default:
					ASSERT(FALSE, "bad reqno %d", req.no);
					ReplyInt(tid, IOSERVER_ERROR_BADREQNO);
					break;
			}
		} else {
			ASSERT(FALSE, "bad data");
			ReplyInt(tid, IOSERVER_ERROR_BADDATA);
		}
	}
}

static inline void init(ioserver_arg *args) {
	int channel = args->channel;
	uart_fifo(channel, args->fifo);
	uart_speed(channel, args->speed);
	uart_stopbits(channel, args->stopbits);
	uart_databits(channel, args->databits);
	uart_parity(channel, args->parity);

	VMEM(UART_BASE(channel) + UART_CTLR_OFFSET) |= TIEN_MASK; // tx int
	VMEM(UART_BASE(channel) + UART_CTLR_OFFSET) |= RIEN_MASK; // rx int
	VMEM(UART_BASE(channel) + UART_CTLR_OFFSET) |= MSIEN_MASK; // mse int
	VMEM(UART_BASE(channel) + UART_INTR_OFFSET) |= 1; // clear mse int
}

//static inline void handle_rx(ioserver_state *state, int channel) {
//	char c = txchar(channel);
//	ReplyNull(state->tid_notifier_rx);
//
//	if (queue_empty(state->input_blocked)) {
//		queue_push(state->input, (void*) c);
//	} else {
//		int tid = (int) queue_pop(state->input_blocked);
//		ReplyInt(tid, c);
//	}
//}
//
//static inline void handle_tx(ioserver_state *state, int channel) {
//	ReplyNull(state->tid_notifier_tx);
//}
static inline void handle_general(ioserver_state *state, int channel) {
	int uartbase = UART_BASE(channel);
	int uart_isr = VMEM(uartbase + UART_INTR_OFFSET);



	if (uart_isr & UARTRXINTR) { // incoming register full
		ASSERT(VMEM(uartbase + UART_FLAG_OFFSET) & RXFF_MASK, "incoming register empty");
		ASSERT(!queue_full(state->input), "input queue full");
		int c = rxchar(channel);
		if (queue_empty(state->input_blocked)) {
			queue_push(state->input, (void*) c);
		} else {
			int tid = (int) queue_pop(state->input_blocked);
			ReplyInt(tid, c);
		}
	}

	if (uart_isr & UARTTXINTR) { // outgoing register empty
		ASSERT(VMEM(uartbase + UART_FLAG_OFFSET) & TXFE_MASK, "outgoing register full");
		state->tx_empty = TRUE;
		VMEM(UART_BASE(channel) + UART_CTLR_OFFSET) &= ~TIEN_MASK;
	}

	if (uart_isr & UARTMSINTR) { // modem status interrupt
		ASSERT(channel != COM2, "COM2 received MSI");
		state->cts = VMEM(uartbase + UART_FLAG_OFFSET) & CTS_MASK;
		VMEM(uartbase + UART_INTR_OFFSET) = 1; // clear mse int
	}

	if (state->tx_empty && state->cts && !queue_empty(state->output)) {
		txchar(state, channel);
	}

	ReplyNull(state->tid_notifier_general);
}

static inline char rxchar(int channel) {
	CHECK_COM(channel);
	int uartbase = UART_BASE(channel);
	return (char)(VMEM(uartbase + UART_DATA_OFFSET) & DATA_MASK);
}

static inline void txchar(ioserver_state *state, int channel) {
	CHECK_COM(channel);
	ASSERT(!queue_empty(state->output), "output empty");
	int uartbase = UART_BASE(channel);
	VMEM(uartbase + UART_DATA_OFFSET) = (int) queue_pop(state->output);
	VMEM(uartbase + UART_CTLR_OFFSET) |= TIEN_MASK; // tx int
	VMEM(uartbase + UART_INTR_OFFSET) = 1; // clear mse int
	state->cts = channel == COM2;
	state->tx_empty = VMEM(uartbase + UART_CTLR_OFFSET) & TXFE_MASK;
}

static inline void handle_getc(ioserver_state *state, int tid) {
	if (queue_empty(state->input)) {
		queue_push(state->input_blocked, (void*) tid);
	} else {
		int c = (int) queue_pop(state->input);
		ReplyInt(tid, c);
	}
}

static inline void handle_putc(ioserver_state *state, int tid, int channel, char c) {
	CHECK_COM(channel);
	if (UNLIKELY(queue_full(state->output))) {
		ERROR("queue full, channel: %d, char: %c (%x)", channel, c, c);
	}
	queue_push(state->output, (void*)(int) c);
	if (state->tx_empty && state->cts) {
		txchar(state, channel);
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
	int tid_ioserver = Create(PRIORITY_IOSERVER, &ioserver);
	if (tid_ioserver < 0) return tid_ioserver;
	int rv;
	int len = Send(tid_ioserver, (char*) &args, sizeof args, (char*) &rv, sizeof rv);
	if (len < 0) return len;
	return rv;
}

static inline int ioserver_send(int channel, ioserver_req *req) {
	CHECK_COM(channel);
	char *name = (channel == COM1) ? NAME_IOSERVER_COM1 : NAME_IOSERVER_COM2;
	int server = WhoIs(name);
	if (server < 0) return server;
	int rv;
	int len = Send(server, (char*) req, sizeof *req, (char*) &rv, sizeof rv);
	if (len < 0) return len;
	return rv;
}

int ioserver_getc(int channel) {
	ioserver_req req;
	req.no = GETC;
	req.channel = channel;
	return ioserver_send(channel, &req);
}

int ioserver_putc(int channel, char c) {
	ioserver_req req;
	req.no = PUTC;
	req.channel = channel;
	req.c = c;
	return ioserver_send(channel, &req);
}


void ioputc(int channel, char c) {
	CHECK_COM(channel);
	Putc(channel, c);
}

void ioputx(int channel, char c) {
	ioputc(channel, char2hex(c >> 4)); // c / 16
	ioputc(channel, char2hex(c & 0xF)); // c % 16
}

void ioputr(int channel, uint reg) {
	char *ch = (char*) &reg;
	ioputx(channel, ch[3]);
	ioputx(channel, ch[2]);
	ioputx(channel, ch[1]);
	ioputx(channel, ch[0]);
	ioputc(channel, ' ');
}

void ioputstr(int channel, char *str) {
	while (*str) ioputc(channel, *str++);
}

void ioputw(int channel, int n, char fc, char *bf) {
	char ch;
	char *p = bf;
	while (*p++ && n > 0) n--;
	while (n-- > 0) ioputc(channel, fc);
	while ((ch = *bf++)) ioputc(channel, ch);
}

int iogetc(int channel) {
	CHECK_COM(channel);
	int base = UART_BASE(channel);
	vmemptr flags = (memptr) (base + UART_FLAG_OFFSET);
	while (!(*flags & RXFF_MASK));
	return (char) VMEM(base + UART_DATA_OFFSET);
}

char ioa2i(char ch, char **src, int base, int *nump) { // only for ioformat
	int num, digit;
	char *p;
	p = *src;
	num = 0;
	while ((digit = char2digit(ch)) >= 0) {
		if (digit > base) break;
		num = num * base + digit;
		ch = *p++;
	}
	*src = p;
	*nump = num;
	return ch;
}

void ioformat(int channel, char *fmt, va_list va) {
	CHECK_COM(channel);
	char bf[12];
	char ch, lz;
	int w;
	while ((ch = *(fmt++))) {
		if (ch != '%') {
			ioputc(channel, ch);
		} else {
			lz = 0;
			w = 0;
			ch = *(fmt++);
			switch (ch) {
				case '0':
					lz = 1;
					ch = *(fmt++);
					break;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					ch = ioa2i(ch, &fmt, 10, &w);
					break;
			}
			switch (ch) {
				case 0:
					return;
				case 'c':
					ioputc(channel, va_arg( va, char ));
					break;
				case 's':
					ioputw(channel, w, 0, va_arg( va, char* ));
					break;
				case 'u':
					uint2str(va_arg( va, uint ), 10, bf);
					ioputw(channel, w, lz, bf);
					break;
				case 'd':
					int2str(va_arg( va, int ), bf);
					ioputw(channel, w, lz, bf);
					break;
				case 'x':
					uint2str(va_arg( va, uint ), 16, bf);
					ioputc(channel, '0');
					ioputc(channel, 'x');
					ioputw(channel, w, lz, bf);
					break;
				case '%':
					ioputc(channel, ch);
					break;
			}
		}
	}
}

void ioprintf(int channel, char *fmt, ... ) {
	CHECK_COM(channel);
	va_list va;
	va_start(va,fmt);
	ioformat( channel, fmt, va );
	va_end(va);
}
