#include <stdio.h>
#include <util.h>



static void uint2str(uint num, uint base, char *bf) {
	int n = 0;
	int dgt;
	uint d = 1;

	while ((num / d) >= base) d *= base;
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

static void int2str(int num, char *bf) {
	if (num < 0) {
		num = -num;
		*bf++ = '-';
	}
	uint2str(num, 10, bf);
}


static int char2digit(char ch) {
	if (ch >= '0' && ch <= '9') return ch - '0';
	if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
	if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
	return -1;
}


static inline int putw(char *outbuf, int n, char fc, char *bf) {
	char *orig_buf = outbuf;
	char ch;
	char *p = bf;
	while (*p++ && n > 0) n--;
	while (n-- > 0) *outbuf++ = fc;
	while ((ch = *bf++)) *outbuf++ = ch;
	return outbuf - orig_buf;
}


static inline char a2i(char ch, char const **src, int base, int *nump) { // only for bwformat
	int num, digit;
	char const *p;
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

static inline int bwformat(char *buf, char const *fmt, va_list va) {
	char * const orig_buf = buf;
	char bf[12];
	char ch, lz;
	int w;
	while ((ch = *(fmt++))) {
		if (ch != '%') {
			*buf++ = ch;
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
					ch = a2i(ch, &fmt, 10, &w);
					break;
			}
			switch (ch) {
				case 0:
					goto done;
				case 'c':
					*buf++ = va_arg( va, char );
					break;
				case 's':
					buf += putw(buf, w, 0, va_arg( va, char* ));
					break;
				case 'u':
					uint2str(va_arg( va, uint ), 10, bf);
					buf += putw(buf, w, lz, bf);
					break;
				case 'd':
					int2str(va_arg( va, int ), bf);
					buf += putw(buf, w, lz, bf);
					break;
				case 'x':
					uint2str(va_arg( va, uint ), 16, bf);
					*buf++ = '0';
					*buf++ = 'x';
					buf += putw(buf, w, lz, bf);
					break;
				case '%':
					*buf++ = ch;
					break;
			}
		}
	}

	done:
	return buf - orig_buf;
}

int sprintf(char *buf, const char *fmt, ... ) {
	ASSERT(buf, "invalid buffer");
	char const * const orig_buf = buf;
	va_list va;
	va_start(va,fmt);
	buf += bwformat(buf, fmt, va );
	va_end(va);

	*buf++ = '\0';
	return buf - orig_buf;
}
