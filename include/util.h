#pragma once

////////// VAR ARG
typedef char *va_list;

#define __va_argsiz(t)  \
        (((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)  ((void)0)

#define va_arg(ap, t)   \
		         (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

//////////// MACHINE STUFF
#define COM1    0
#define COM2    1
#define COM_COUNT 2

#define UART_BASE(_x) (((_x) == COM1) ? UART1_BASE : UART2_BASE)

#define ON  1
#define OFF 0

///////////// USEFUL MACROS
#define TRUE 1
#define FALSE 0
#define NULL 0
#define CRLF "\r\n"
typedef volatile unsigned int *memptr;
#define MEM(x) (*(memptr)(x))
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define MAX(x, y) ( ( (x) > (y) ) ? (x) : (y) )
#define MIN(x, y) ( ( (x) < (y) ) ? (x) : (y) )
#define INSTALL_INTERRUPT_HANDLER(vec, jmp) { MEM((vec) + 0x20) = (unsigned int)(jmp); }
#define BIT_TOGGLE(word, mask, flag) ((word) ^= (-(flag) ^ (word)) & (mask)) // turn mask bits on/off in word based on flag (improve with orr/bic?)
///////////// DEBUG
#define ASSERT_ENABLED 1
#define DEBUG 0

void errormsg(char*);
void die();

#if ASSERT_ENABLED
#define ASSERT(X, MSG) \
{\
	if (!(X)) {\
		errormsg("assertion failed in function " __FILE__ " line:" TOSTRING(__LINE__) CRLF);\
		errormsg(MSG); die(); \
	}\
}
#else
#define ASSERT(X,MSG) { }
#endif

#define CHECK_COM(_c) ASSERT((_c) == COM1 ||  (_c) == COM2, "Invalid channel " #_c)

#if _X86_
#define ASM(X)
#else
#define ASM(X) __asm(X)
#endif
