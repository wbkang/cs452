#pragma once

#include <util.h>
#include <stdio.h>

#define FIXED_Q 12 // corresponds to ~3.61 correct decimal digits
#define FIXED_K (1 << (FIXED_Q - 1))
#define FIXED_10toA 10000 // 10^floor(log10(2^Q - 1)) -- num frac digits to print

#define FIXED_MAX_N ((1 << (32 - FIXED_Q - 1)) - 1)
#define FIXED_MIN_N (-1 << (32 - FIXED_Q - 1))

#define fixed_iszero(f) ((f).v == 0)
#define fixed_comp(a,b) ((a).v - (b).v)
#define fixed_sign(f) ((f).v)

typedef struct {int v;} fixed;

static inline fixed fixed_new(int n) {
	ASSERT(n >= FIXED_MIN_N, "n too small. n: %d", n);
	ASSERT(n <= FIXED_MAX_N, "n too big n: %d", n);
	fixed rv = { n << FIXED_Q };
	return rv;
}

static inline int fixed_int(fixed f) {
	int n = f.v;
	if (n < 0) {
		n = -n;
	}
	n >>= FIXED_Q;
	return n;
}

static inline int fixed_fra(fixed f) {
	int n = f.v;
	if (n < 0) {
		n = -n;
	}
	return n & ((1 << FIXED_Q) - 1);
}

static inline int fixed_print(char *buf, fixed f) {
	int n = f.v;
    char *b = buf;
	if (n < 0) {
		*b++ = '-';
	}
	b += sprintf(b, "%d", fixed_int(f));
	*b++ = '.';
	int fra = ((long long) fixed_fra(f) * FIXED_10toA) >> FIXED_Q;
	int q;
	for (q = FIXED_10toA / 10; q; q /= 10) {
		int d = fra / q;
		fra -= d * q;
		*b++ = '0' + d;
	}
	*b = '\0';
	return b - buf;
}

static inline fixed fixed_add(fixed fa, fixed fb) {
	int a = fa.v; int b = fb.v;
	ASSERT(!overflow(a, b), "overflow. a: %F, b: %F", fa, fb);
	fixed rv = { a + b };
	return rv;
}

static inline fixed fixed_sub(fixed fa, fixed fb) {
	int a = fa.v; int b = fb.v;
	ASSERT(!overflow(a, -b), "underflow a: %F, b: %F", fa, fb);
	fixed rv = { a - b };
	return rv;
}

// @TODO: add overflow assert
static inline fixed fixed_mul(fixed fa, fixed fb) {
	int a = fa.v; int b = fb.v;
	long long tmp = (long long) a * (long long) b;
	tmp += FIXED_K; // round
	fixed rv = { tmp >> FIXED_Q };
	return rv;
}

// @TODO: add overflow assert
static inline fixed fixed_div(fixed fa, fixed fb) {
	int a = fa.v; int b = fb.v;
	ASSERT(b != 0, "div by zero. a: %F, b: %F", fa, fb);
	long long tmp = ((long long) a) << FIXED_Q;
	tmp += b >> 1; // round
	fixed rv = { tmp / b };
	return rv;
}
