#pragma once

#include <util.h>
#include <stdio.h>

#define FIXED_Q 14 // corresponds to log10(2^Q) correct decimal digits
#define FIXED_K (1 << (FIXED_Q - 1))
#define FIXED_10toA 1000 // 10^floor(log10(2^Q - 1)) -- num frac digits to print

#define FIXED_MAX_N ((1 << (32 - FIXED_Q - 1)) - 1)
#define FIXED_MIN_N (-1 << (32 - FIXED_Q - 1))

typedef struct {
	int v;
} fixed;

static inline fixed fixed_new(int n) {
	ASSERT(n >= FIXED_MIN_N, "n too small. n: %d", n);
	ASSERT(n <= FIXED_MAX_N, "n too big n: %d", n);
	return (fixed) {n << FIXED_Q};
}

static inline int fixed_int(fixed f) {
	return f.v >>= FIXED_Q;
}

static inline int fixed_fra(fixed f) {
	int n = f.v;
	if (n < 0) {
		n = -n;
	}
	return n & ((1 << FIXED_Q) - 1);
}

static inline int fixed_print(char *buf, fixed f) {
    char *b = buf;
	if (f.v < 0) {
		*b++ = '-';
		f.v = -f.v;
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
	int a = fa.v;
	int b = fb.v;
	ASSERT(!overflow(a, b), "overflow. a: %F, b: %F", fa, fb);
	return (fixed) {a + b};
}

static inline fixed fixed_sub(fixed fa, fixed fb) {
	int a = fa.v;
	int b = fb.v;
	ASSERT(!overflow(a, -b), "underflow a: %F, b: %F", fa, fb);
	return (fixed) {a - b};
}

static inline fixed fixed_mul(fixed fa, fixed fb) {
	int a = fa.v;
	int b = fb.v;
	long long tmp = (long long) a * (long long) b;
	tmp += FIXED_K; // round
	tmp >>= FIXED_Q;
	ASSERT(INT_MIN <= tmp && tmp <= INT_MAX, "overflow. a: %F, b: %F", fa, fb);
	return (fixed) {tmp};
}

static inline fixed fixed_div(fixed fa, fixed fb) {
	int a = fa.v;
	int b = fb.v;
	ASSERT(b != 0, "div by zero. a: %F, b: %F", fa, fb);
	long long tmp = ((long long) a) << FIXED_Q;
	tmp += b >> 1; // round
	tmp /= b;
	ASSERT(INT_MIN <= tmp && tmp <= INT_MAX, "overflow. a: %F, b: %F", fa, fb);
	return (fixed) {tmp};
}

static inline int fixed_sgn(fixed f) {
	return f.v;
}

static inline int fixed_is0(fixed f) {
	return f.v == 0;
}

static inline int fixed_cmp(fixed a, fixed b) {
	return fixed_sgn(fixed_sub(a, b));
}

static inline fixed fixed_abs(fixed f) {
	if (fixed_sgn(f) < 0) {
		f.v = -f.v;
	}
	return f;
}

static inline fixed fixed_min(fixed a, fixed b) {
	return a.v < b.v ? a : b;
}

static inline fixed fixed_neg(fixed f) {
	f.v = -f.v;
	return f;
}

static inline fixed fixed_eps() {
	return (fixed) {1};
}
