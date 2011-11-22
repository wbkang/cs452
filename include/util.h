#pragma once

#include <ts7200.h>
#include <constants.h>

////////// TYPES
typedef unsigned int uint;
typedef uint *memptr;
typedef uint volatile *volatile vmemptr;
typedef void (*func_t)();
typedef int (*hash_fn)(void* data);

////////// .c
uint random();

////////// VAR ARG
typedef char *va_list;
#define __va_argsiz(t) (((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))
#define va_end(ap)  ((void)0)
#define va_arg(ap, t) (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

///////////// USEFUL MACROS
#define VMEM(x) (*(vmemptr)(x))
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define INSTALL_INTERRUPT_HANDLER(vec, jmp) { VMEM((vec) + 0x20) = (uint)(jmp); }
#define READ_INTERRUPT_HANDLER(vec) VMEM((vec) + 0x20)
// turn mask bits on/off in word based on flags
#define BIT_TOGGLE(word, mask, flag) ((word) ^= (-(flag) ^ (word)) & (mask))
#define BYTES2WORDS(x) ((x) >> 2)
#define WORDS2BYTES(x) ((x) << 2)
#define ALIGN(x, boundary) ((x + boundary - 1) & ~(boundary - 1))
#define NEXTHIGHESTWORD(x) BYTES2WORDS((x) + 3)
#define LIKELY(x) __builtin_expect((x), 1)
#define UNLIKELY(x) __builtin_expect((x), 0)
#define INT_MIN ((int)0x80000000)
#define INT_MAX 0x7fffffff
#define MEMCHECK() malloc(0)

#define PUTS(...) { \
	char PAVEL[512]; \
	sprintf(PAVEL, __VA_ARGS__); \
	int tid_com2 = WhoIs(NAME_IOSERVER_COM2); \
	ASSERT(tid_com2 >= 0, "bad io server"); \
	int rv = Putstr(COM2, PAVEL, tid_com2); \
	ASSERT(rv >= 0, "putstr error"); \
}

static inline int log2(uint n) {
	uint l, i = 0;
	if ((l = n >> 16)) { n = l; i |= 16; }
	if ((l = n >>  8)) { n = l; i |=  8; }
	if ((l = n >>  4)) { n = l; i |=  4; }
	if ((l = n >>  2)) { n = l; i |=  2; }
	if ((l = n >>  1)) { n = l; i |=  1; }
	return i;
}

static inline void uptime_reset() {
	VMEM(0x80810064) &= ~0x100;
	VMEM(0x80810064) |= 0x100;
}

static inline void uptime_teardown() {
	VMEM(0x80810064) &= ~0x100;
}

// will overflow every 2^s * (2^32-1) / (983 kilohertz) = 1.2 hours * 2^s
static inline uint uptime() {
	int s = 0;
    return ((VMEM(0x80810064) & 0xff) << (32 - s)) | (VMEM(0x80810060) >> s); // timer4
}

static inline int min(int a, int b) {
	return a < b ? a : b;
}

static inline int max(int a, int b) {
	return a < b ? b : a;
}

static inline uint abs(int n) {
	int const mask = n >> 31;
	return (n + mask) ^ mask;
}

static inline int overflow(int x, int y) {
	return (y > 0 && x > INT_MAX - y) || (y < 0 && x < INT_MIN - y);
}

///////////// DEBUG
#define MEM_PROTECTION 0 // MEM_CACHE must be on
#define MEM_CACHE 1
#define ASSERT_ENABLED 1
#define TRACE_ENABLED 1
#define TEST_ENABLED 1
#define KERNELTEST_ENABLED 0

#define TEST_REALLY_ENABLED (ASSERT_ENABLED && TEST_ENABLED)
#define KTEST_REALLY_ENABLED (ASSERT_ENABLED && KERNELTEST_ENABLED)
#define LONG_TEST_ENABLED 0
void bwprintf(int channel, char *fmt, ...);

void errormsg(char *msg);
void die();

#define WRITE_REGISTER(var) __asm volatile("mov " TOSTRING(var) ", %[" TOSTRING(var) "]" "\n\t" : : [var] "r" (var))
#define READ_REGISTER(var) __asm volatile("mov %[" TOSTRING(var) "], " TOSTRING(var) "\n\t" : [var] "=r" (var))
#define READ_CPSR(var) __asm("mrs %[mode], cpsr" "\n\t"	"and %[mode], %[mode], #0x1f" "\n\t" : [mode] "=r" (var))
void Exit();
int main();
void dump_registers(int r0, int r1, int r2, int r3);
void print_stack_trace(uint fp, int clearscreen);

#if ASSERT_ENABLED
#define ASSERT(X, ...) { \
	if (!(X)) { /*__asm("swi 12\n\t");*/ \
		VMEM(VIC1 + INTENCLR_OFFSET) = ~0; \
		VMEM(VIC2 + INTENCLR_OFFSET) = ~0; \
		bwprintf(0, "%c", 0x61); \
		int fp, lr, pc; READ_REGISTER(fp); READ_REGISTER(lr); READ_REGISTER(pc); \
		/*bwprintf(1, "\x1B[2J" "\x1B[1;1H");*/ \
		bwprintf(1, "assertion failed in file " __FILE__ " line:" TOSTRING(__LINE__) " lr: %x pc: %x" CRLF, lr, pc); \
		bwprintf(1, "[%s] ", __func__); \
		bwprintf(1, __VA_ARGS__); \
		bwprintf(1, "\n"); \
		print_stack_trace(fp, 0); \
		die(); \
	} \
}
#else
#define ASSERT(X,...)
#endif

#define ASSERTNOTNULL(X) ASSERT((X), TOSTRING(X) " is null")

#define ERROR(...) { \
	bwprintf(1, "ERROR:" __VA_ARGS__); \
	bwprintf(1, CRLF "File: " __FILE__ " line: " TOSTRING(__LINE__) CRLF); \
	bwprintf(1, "\n"); \
	die(); \
}

#if TRACE_ENABLED
void bwprintf(int channel, char *fmt, ...);
#define TRACE(...) { \
	bwprintf(1, "[%s] ", __func__); \
	bwprintf(1, __VA_ARGS__); \
	bwprintf(1, "\n"); \
}
#else
#define TRACE(...)
#endif

#define PRINT(...) { \
	int __vic1 = VMEM(VIC1 + INTENCLR_OFFSET); \
	int __vic2 = VMEM(VIC2 + INTENCLR_OFFSET); \
	VMEM(VIC1 + INTENCLR_OFFSET) = ~0; \
	VMEM(VIC2 + INTENCLR_OFFSET) = ~0; \
	bwprintf(1, "[%s] ", __func__); \
	bwprintf(1, __VA_ARGS__); \
	bwprintf(1, "\n"); \
	VMEM(VIC1 + INTENCLR_OFFSET) = __vic1; \
	VMEM(VIC2 + INTENCLR_OFFSET) = __vic2; \
}

#define CHECK_COM(_c) ASSERT((_c) == COM1 ||  (_c) == COM2, "Invalid channel " #_c)

#if __i386__
#define ASM(X)
#else
#define ASM(X) __asm(X)
#endif
