#include <rawio.h>
#include <util.h>
#include <ts7200.h>
#include <string.h>
#include <hardware.h>
#include <fixed.h>

void raw_init() {
	// init COM1
	uart_fifo(COM1, OFF);
	uart_speed(COM1, 2400);
	uart_stopbits(COM1, 2);
	uart_databits(COM1, 8);
	uart_parity(COM1, OFF);
	// init COM2
	uart_fifo(COM2, OFF);
}

void bwputc(int channel, char c) {
	CHECK_COM(channel);
	int base = UART_BASE(channel);
	vmemptr flags = (vmemptr) (base + UART_FLAG_OFFSET);
	while (*flags & TXFF_MASK);
	VMEM(base + UART_DATA_OFFSET) = c;
}

void bwputx(int channel, char c) {
	bwputc(channel, char2hex(c >> 4)); // c / 16
	bwputc(channel, char2hex(c & 0xF)); // c % 16
}

void bwputr(int channel, uint reg) {
	char *ch = (char*) &reg;
	bwputx(channel, ch[3]);
	bwputx(channel, ch[2]);
	bwputx(channel, ch[1]);
	bwputx(channel, ch[0]);
	bwputc(channel, ' ');
}

void bwputstr(int channel, char *str) {
	while (*str) bwputc(channel, *str++);
}

void bwputw(int channel, int n, char fc, char *bf) {
	char ch;
	char *p = bf;
	while (*p++ && n > 0) n--;
	while (n-- > 0) bwputc(channel, fc);
	while ((ch = *bf++)) bwputc(channel, ch);
}

int bwgetc(int channel) {
	CHECK_COM(channel);
	int base = UART_BASE(channel);
	vmemptr flags = (memptr) (base + UART_FLAG_OFFSET);
	while (!(*flags & RXFF_MASK));
	return (char) VMEM(base + UART_DATA_OFFSET);
}

static char bwa2i(char ch, char **src, int base, int *nump) { // only for bwformat
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

void bwformat(int channel, char *fmt, va_list va) {
	CHECK_COM(channel);
	char bf[32 + 1];
	char ch, lz;
	int w;
	while ((ch = *(fmt++))) {
		if (ch != '%') {
			bwputc(channel, ch);
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
					ch = bwa2i(ch, &fmt, 10, &w);
					break;
			}
			switch (ch) {
				case '\0':
					return;
				case 'c':
					bwputc(channel, va_arg( va, char ));
					break;
				case 's':
					bwputw(channel, w, 0, va_arg( va, char* ));
					break;
				case 'u':
					uint2str(va_arg( va, uint ), 10, bf);
					bwputw(channel, w, lz, bf);
					break;
				case 'd':
					int2str(va_arg( va, int ), bf);
					bwputw(channel, w, lz, bf);
					break;
				case 'b':
					uint2str(va_arg( va, uint ), 2, bf);
					bwputc(channel, '0');
					bwputc(channel, 'b');
					bwputw(channel, w, lz, bf);
					break;
				case 'x':
					uint2str(va_arg( va, uint ), 16, bf);
					bwputc(channel, '0');
					bwputc(channel, 'x');
					bwputw(channel, w, lz, bf);
					break;
				case 'F': {
					fixed_print(bf, va_arg(va, fixed));
					bwputstr(COM2, bf);
					break;
				}
				case '%':
					bwputc(channel, ch);
					break;
			}
		}
	}
}

void bwprintf(int channel, char *fmt, ...) {
	CHECK_COM(channel);
	va_list va;
	va_start(va, fmt);
	bwformat(channel, fmt, va);
	va_end(va);
}
