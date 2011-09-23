#include <util.h>
#include <ts7200.h>

int raw_isrxempty(int channel) {
	CHECK_COM(channel);
	return MEM(UART_BASE(channel) + UART_FLAG_OFFSET) & RXFE_MASK;
}

int raw_getc(int channel) {
	CHECK_COM(channel);
	return MEM(UART_BASE(channel) + UART_DATA_OFFSET);
}

int raw_istxready(int channel) {
	CHECK_COM(channel);
	int flags = MEM(UART_BASE(channel) + UART_FLAG_OFFSET);
	return !(flags & TXFF_MASK)
			&& (channel == COM2 || (channel == COM1 && (flags & CTS_MASK)));
}

void raw_putc(int channel, char c) {
	CHECK_COM(channel);
	MEM(UART_BASE(channel) + UART_DATA_OFFSET) = c;
}

void bwputc(int channel, char c) {
	CHECK_COM(channel);
	int base = UART_BASE(channel);
	memptr flags = (memptr) (base + UART_FLAG_OFFSET);
	while (*flags & TXFF_MASK)
		;
	MEM(base + UART_DATA_OFFSET) = c;
}

char c2x(char ch) {
	return (ch < 10 ? '0' + ch : 'a' + ch - 10);
}

void bwputx(int channel, char c) {
	bwputc(channel, c2x(c / 16));
	bwputc(channel, c2x(c % 16));
}

void bwputr(int channel, unsigned int reg) {
	char *ch = (char *) &reg;
	bwputx(channel, ch[3]);
	bwputx(channel, ch[2]);
	bwputx(channel, ch[1]);
	bwputx(channel, ch[0]);
	bwputc(channel, ' ');
}

void bwputstr(int channel, char *str) {
	while (*str)
		bwputc(channel, *str++);
}

void bwputw(int channel, int n, char fc, char *bf) {
	char ch;
	char *p = bf;
	while (*p++ && n > 0)
		n--;
	while (n-- > 0)
		bwputc(channel, fc);
	while ((ch = *bf++))
		bwputc(channel, ch);
}

int bwgetc(int channel) {
	CHECK_COM(channel);
	int base = UART_BASE(channel);
	memptr flags = (memptr) (base + UART_FLAG_OFFSET);
	while (!(*flags & RXFF_MASK))
		;
	return (char) MEM(base + UART_DATA_OFFSET);
}

int bwa2d(char ch) {
	if (ch >= '0' && ch <= '9') return ch - '0';
	if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
	if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
	return -1;
}

char bwa2i(char ch, char **src, int base, int *nump) {
	int num, digit;
	char *p;

	p = *src;
	num = 0;
	while ((digit = bwa2d(ch)) >= 0) {
		if (digit > base) break;
		num = num * base + digit;
		ch = *p++;
	}
	*src = p;
	*nump = num;
	return ch;
}

void bwui2a(unsigned int num, unsigned int base, char *bf) {
	int n = 0;
	int dgt;
	unsigned int d = 1;

	while ((num / d) >= base)
		d *= base;
	while (d != 0) {
		dgt = num / d;
		num %= d;
		d /= base;
		if (n || dgt > 0 || d == 0) {
			*bf++ = dgt + (dgt < 10 ? '0' : 'a' - 10);
			++n;
		}
	}
	*bf = 0;
}

void bwi2a(int num, char *bf) {
	if (num < 0) {
		num = -num;
		*bf++ = '-';
	}
	bwui2a(num, 10, bf);
}

void bwformat(int channel, char *fmt, va_list va) {
	CHECK_COM(channel);
	char bf[12];
	char ch, lz;
	int w;

	while ((ch = *(fmt++))) {
		if (ch != '%') bwputc(channel, ch);
		else {
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
				case 0:
					return;
				case 'c':
					bwputc(channel, va_arg( va, char ));
					break;
				case 's':
					bwputw(channel, w, 0, va_arg( va, char* ));
					break;
				case 'u':
					bwui2a(va_arg( va, unsigned int ), 10, bf);
					bwputw(channel, w, lz, bf);
					break;
				case 'd':
					bwi2a(va_arg( va, int ), bf);
					bwputw(channel, w, lz, bf);
					break;
				case 'x':
					bwui2a(va_arg( va, unsigned int ), 16, bf);
					bwputw(channel, w, lz, bf);
					break;
				case '%':
					bwputc(channel, ch);
					break;
			}
		}
	}
}

void bwprintf(int channel, char *fmt, ... ) {
	CHECK_COM(channel);
	va_list va;
	va_start(va,fmt);
	bwformat( channel, fmt, va );
	va_end(va);
}
