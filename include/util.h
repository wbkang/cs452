#pragma once

#include <ts7200.h>
#include <constants.h>

////////// TYPES

typedef unsigned int uint;
typedef uint *memptr;
typedef uint volatile * volatile vmemptr;
typedef void (*func_t)();

////////// .c

uint random();

////////// VAR ARG
typedef char *va_list;

#define __va_argsiz(t)  \
        (((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)  ((void)0)

#define va_arg(ap, t)   \
		         (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

///////////// USEFUL MACROS
#define VMEM(x) (*(vmemptr)(x))
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define MAX(x, y) ( ( (x) > (y) ) ? (x) : (y) )
#define MIN(x, y) ( ( (x) < (y) ) ? (x) : (y) )
#define INSTALL_INTERRUPT_HANDLER(vec, jmp) { VMEM((vec) + 0x20) = (uint)(jmp); }
// turn mask bits on/off in word based on flags
#define BIT_TOGGLE(word, mask, flag) ((word) ^= (-(flag) ^ (word)) & (mask))
#define BYTES2WORDS(x) ((x) >> 2)
#define NEXTHIGHESTWORD(x) BYTES2WORDS((x) + 3)
// 2k clock, divide by 2 to get time in ms, with 6 ticks per 2000 ticks drift correction
#define GET_TIME(time) { \
    time = ~VMEM(TIMER3_BASE + VAL_OFFSET); \
    time += (3 * time) / 1000; \
    time >>= 1; \
}

///////////// DEBUG
#define ASSERT_ENABLED 1
#define TRACE_ENABLED 0
#define TEST_ENABLED 1
#define LONG_TEST_ENABLED 0
void bwprintf(int channel, char *fmt, ... );

void errormsg(char*);
void die();

#if ASSERT_ENABLED
#define ASSERT(X, ...) { \
	if (!(X)) { \
		errormsg("assertion failed in file " __FILE__ " line:" TOSTRING(__LINE__) CRLF); \
		bwprintf(1, "[%s] ", __func__); \
		bwprintf(1, __VA_ARGS__); \
		bwprintf(1, "\n"); \
		die(); \
	} \
}
#else
#define ASSERT(X,...)
#endif

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
	bwprintf(1, "[%s] ", __func__); \
	bwprintf(1, __VA_ARGS__); \
	bwprintf(1, "\n"); \
}

#define CHECK_COM(_c) ASSERT((_c) == COM1 ||  (_c) == COM2, "Invalid channel " #_c)

#if __i386__
#define ASM(X)
#else
#define ASM(X) __asm(X)
#endif
