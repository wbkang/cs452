#pragma once

#include <util.h>
#include <stdio.h>

#define FIXED_Q 14 // corresponds to 4 correct decimal digits
#define FIXED_K (1 << (FIXED_Q - 1))
#define FIXED_10toA 10000 // 10^floor(log10(2^Q - 1)) -- num frac digits to print

#define FIXED_MAX_N ((1 << (32 - 1 - FIXED_Q)) - 1)
#define FIXED_MIN_N (-1 << (32 - 1 - FIXED_Q))

typedef int fixed;

static inline int fixed_new(int n) {
	ASSERT(n >= FIXED_MIN_N, "n too small");
	ASSERT(n <= FIXED_MAX_N, "n too big");
	return n << FIXED_Q;
}

static inline int fixed_int(fixed n) {
	if (n < 0) {
		n = -n;
	}
	return n >> FIXED_Q;
}

static inline int fixed_fra(fixed n) {
	if (n < 0) {
		n = -n;
	}
	return n & ((1 << FIXED_Q) - 1);
}

static inline int fixed_print(char *buf, fixed n) {
    char *b = buf;
	if (n < 0) {
		*b++ = '-';
	}
	b += sprintf(b, "%d", fixed_int(n));
	*b++ = '.';
	int fra = ((long long) fixed_fra(n) * FIXED_10toA) >> FIXED_Q;
	int q;
	for (q = FIXED_10toA / 10; q; q /= 10) {
		int d = fra / q;
		fra -= d * q;
		*b++ = '0' + d;
	}
	*b = '\0';
	return b - buf;
}

static inline fixed fixed_add(fixed a, fixed b) {
	ASSERT(!overflow(a, b), "overflow");
	return a + b;
}

static inline fixed fixed_sub(fixed a, fixed b) {
	ASSERT(!overflow(a, -b), "underflow");
	return a - b;
}

// @TODO: add overflow assert
static inline fixed fixed_mul(fixed a, fixed b) {
	long long tmp = (long long) a * (long long) b;
	tmp += FIXED_K; // round
	return tmp >> FIXED_Q;
}

// @TODO: add overflow assert
static inline fixed fixed_div(fixed a, fixed b) {
	ASSERT(b != 0, "div by zero");
	long long tmp = ((long long) a) << FIXED_Q;
	tmp += b >> 1; // round
	return tmp / b;
}
