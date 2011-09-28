#pragma once

////////// TYPES

typedef unsigned int uint;
typedef uint *memptr;
typedef void (*func_t)();

////////// VAR ARG
typedef char *va_list;

#define __va_argsiz(t)  \
        (((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)  ((void)0)

#define va_arg(ap, t)   \
		         (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

///////////// USEFUL MACROS
#define TRUE 1
#define FALSE 0
#define NULL 0
#define CRLF "\r\n"
#define MEM(x) (*(memptr)(x))
#define VMEM(x) (*(volatile memptr)(x))
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define MAX(x, y) ( ( (x) > (y) ) ? (x) : (y) )
#define MIN(x, y) ( ( (x) < (y) ) ? (x) : (y) )
#define INSTALL_INTERRUPT_HANDLER(vec, jmp) { VMEM((vec) + 0x20) = (uint)(jmp); }
#define ROUND_UP(x, num) ((((uint)x)+(num-1))&~(num-1))
// turn mask bits on/off in word based on flag (improve with orr/bic?)
#define BIT_TOGGLE(word, mask, flag) ((word) ^= (-(flag) ^ (word)) & (mask))
///////////// DEBUG
#define ASSERT_ENABLED 1
#define TRACE_ENABLED 1

void errormsg(char*);
void die();

#if ASSERT_ENABLED
#define ASSERT(X, MSG) { \
	if (!(X)) { \
		errormsg("assertion failed in function " __FILE__ " line:" TOSTRING(__LINE__) CRLF); \
		errormsg(MSG); die(); \
	} \
}
#else
#define ASSERT(X,MSG)
#endif

#if TRACE_ENABLED
void bwprintf(int channel, char *fmt, ... );
#define TRACE(...) { bwprintf(1, __VA_ARGS__); }
#else
#define TRACE(...)
#endif

#define CHECK_COM(_c) ASSERT((_c) == COM1 ||  (_c) == COM2, "Invalid channel " #_c)

#if __i386__
#define ASM(X)
#else
#define ASM(X) __asm(X)
#endif
