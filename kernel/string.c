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

	while ((num / d) >= base) {
		d *= base;
	}
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

uint strlen(char const *str) {
	char const *i = str;
	while (*i) i++;
	return i - str;
}

void strcpy(char *dest, const char *src) {
	while ((*dest++ = *src++));
}

int memcmp(const void *p1, const void *p2, uint count) {
	int cmp = 0;
	char* ptr1 = (char*)p1;
	char* ptr2 = (char*)p2;

	while (count --> 0 && cmp == 0) {
		cmp = *(ptr1++) - *(ptr2++);
	}

	return cmp;
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

uint strgetui(char **c) {
	uint num = 0;
	int digit;
	for (;;) {
		digit = char2digit(**c);
		if (digit < 0 || digit > 10) break;
		num = num * 10 + digit;
		(*c)++;
	}
	return num;
}

void *memcpy9(void *dst, void const *src, uint len) {
	if (LIKELY(len > 0)) {
//		ASSERT(((int) src & 3) == 0, "src unaligned: %x", src);
//		ASSERT(((int) dst & 3) == 0, "dst unaligned: %x", src);
//		ASSERT((len & 3) == 0, "length unaligned: %d (%x)", len, len);
		int *to = (int*) dst;
		int const *from = (int const*) src;

	    if ((((int) src | (int) dst) & 3) == 0) { // aligned
		    uint words = BYTES2WORDS(len);
			int n = (words + 7) >> 3;
			switch (words & 7) {
				case 0:	do {	*to++ = *from++;
				case 7:			*to++ = *from++;
				case 6:			*to++ = *from++;
				case 5:			*to++ = *from++;
				case 4:			*to++ = *from++;
				case 3:			*to++ = *from++;
				case 2:			*to++ = *from++;
				case 1:			*to++ = *from++;
						} while (--n > 0);
			}
			len &= 3;
		}

		char *ldst = (char*) to;
		char const *lsrc = (char const*) from;
		while (len--) {
			*ldst++ = *lsrc++;
		}
	}
    return dst;
}

void *memcpy(void *dst, void const *src, uint len) {
	if (len > 0) {
//		ASSERT((len & 3) == 0, "length unaligned: %d (%x)", len, len);
	    if ((((int) src | (int) dst | len) & 3) == 0) { // aligned
		    int *to = (int*) dst;
		    int const *from = (int const*) src;
		    uint words = len >> 2;
			int n = (words + 7) >> 3;
			switch (words & 7) {
				case 0:	do {	*to++ = *from++;
				case 7:			*to++ = *from++;
				case 6:			*to++ = *from++;
				case 5:			*to++ = *from++;
				case 4:			*to++ = *from++;
				case 3:			*to++ = *from++;
				case 2:			*to++ = *from++;
				case 1:			*to++ = *from++;
						} while(--n > 0);
			}
	    } else {
		    char *ldst = (char*) dst;
		    char const *lsrc = (char const*) src;
		    while (len--) {
		        *ldst++ = *lsrc++;
		    }
	    }
	}
    return dst;
}

// bulletproof memcpy
void *memcpy8(void *dst, void const *src, uint len) {
    char *ldst = (char*) dst;
    char const *lsrc = (char const*) src;
    while (len--) {
        *ldst++ = *lsrc++;
    }
    return dst;
}
