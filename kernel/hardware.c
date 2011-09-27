#include <util.h>
#include <ts7200.h>
#include <hardware.h>

void uart_fifo(int channel, int state) {
	CHECK_COM(channel);
	int addr = UART_BASE(channel) + UART_LCRH_OFFSET;
	int buf = VMEM(addr);
	BIT_TOGGLE(buf, FEN_MASK, state);
	VMEM(addr) = buf;
}

void uart_stopbits(int channel, int bits) {
	CHECK_COM(channel);
	int addr = UART_BASE(channel) + UART_LCRH_OFFSET;
	int buf = VMEM(addr);
	BIT_TOGGLE(buf, STP2_MASK, bits == 2);
	VMEM(addr) = buf;
}

void uart_databits(int channel, int wlen) {
	CHECK_COM(channel);
	int addr = UART_BASE(channel) + UART_LCRH_OFFSET;
	int buf = VMEM(addr);
	BIT_TOGGLE(buf, WLEN_MASK, wlen - 5);
	VMEM(addr) = buf;
}

void uart_parity(int channel, int enable) {
	CHECK_COM(channel);
	int addr = UART_BASE(channel) + UART_LCRH_OFFSET;
	int buf = VMEM(addr);
	BIT_TOGGLE(buf, PEN_MASK, enable);
	VMEM(addr) = buf;
}

void uart_speed(int channel, int speed) {
	CHECK_COM(channel);
	ASSERT(speed == 115200 || speed == 2400, "Unsupported speed");
	int base = UART_BASE(channel);
	VMEM(base + UART_LCRM_OFFSET) = 0x0;
	VMEM(base + UART_LCRL_OFFSET) = (speed == 2400 ? 0xBF : 0x3); // F_UARTCLK / (16 * BAUD_RATE) - 1
}

uint mem_start_address() {
	return 0x300000;
}
