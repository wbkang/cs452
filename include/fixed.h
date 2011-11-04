#define FIXED_Q 16
#define FIXED_K (1 << (FIXED_Q - 1))
#define FIXED_A (FIXED_Q * 10) / 33 // accuracy
#define FIXED_10toA 10000

typedef int fixed;

static inline int fixed_new(int inte, int frac) {
	ASSERT(frac >= 0, "bad fractional part");
	ASSERT(frac < (1 << FIXED_Q), "bad fractional part");
	ASSERT(abs(inte) < (1 << (32 - 1 - FIXED_Q)), "bad integer part");
	return (inte << FIXED_Q) | frac;
}

static inline int fixed_int(fixed n) {
	return n >> FIXED_Q;
}

static inline int fixed_fra(char *buf, fixed n) {
	long long f = n & ((1 << FIXED_Q) - 1);
	long long fra = (f * FIXED_10toA) >> FIXED_Q;
	PRINT("(%d)", fra);
	for (int i = 0, q = FIXED_10toA; i < FIXED_A; i++, q /= 10) {
		int d = (fra / q) % 10;
		buf[i] = '0' + d;
	}
	return FIXED_A;
}

static inline fixed fixed_add(fixed a, fixed b) {
	ASSERT(!overflow(a, b), "overflow");
	return a + b;
}

static inline fixed fixed_sub(fixed a, fixed b) {
	ASSERT(!overflow(a, -b), "underflow");
	return a - b;
}

static inline fixed fixed_mul(fixed a, fixed b) {
	long long tmp = (long long) a * (long long) b;
	tmp += FIXED_K; // round
	return tmp >> FIXED_Q;
}

static inline fixed fixed_div(fixed a, fixed b) {
	ASSERT(b, "div by zero!");
	long long tmp = ((long long) a) << FIXED_Q;
	tmp += b >> 1;
	return tmp / b;
}
