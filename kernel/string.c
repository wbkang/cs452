#include <string.h>
#include <util.h>

char char2hex(char n) {
	return (n < 10 ? '0' + n : 'a' + n - 10);
}

int char2digit(char ch) {
	if (ch >= '0' && ch <= '9') return ch - '0';
	if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
	if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
	return -1;
}

void uint2str(uint num, uint base, char *bf) {
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

void int2str(int num, char *bf) {
	if (num < 0) {
		num = -num;
		*bf++ = '-';
	}
	uint2str(num, 10, bf);
}

uint strlen(char *str) {
	char *i = str;
	while (*i) i++;
	return i - str;
}

// @TODO implement this in assembly with ldm and stm
void strcpy(char *dest, const char *src) {
	while ((*dest++ = *src++));
}

uint strparseuint(char *str, int *idx) {
	uint num = 0;
	int digit;
	for (;;) {
		digit = char2digit(str[*idx]);
		if (digit < 0 || digit > 10) break;
		num = num * 10 + digit;
		(*idx)++;
	}
	return num;
}

// @TODO implement this in assembly with ldm and stm
void* memcpy(void* destination, const void* source, uint num) {
	char *dst = (char *) destination;
	char *src = (char *) source;
	while (num--) *dst++ = *src++;
	return destination;
}

