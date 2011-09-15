#ifndef UTIL_H_
#define UTIL_H_
typedef char *va_list;

#define __va_argsiz(t)  \
        (((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
		
#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))
		
#define va_end(ap)  ((void)0)
		
#define va_arg(ap, t)   \
		         (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))
				 
#define COM1    0
#define COM2    1
				 
#define ON  1
#define OFF 0

#define MEM(X) (*(int *)(X))
#define die() {} // TODO
#define ASSERT(X, MSG) { if (!X) { bwputstr(COM2, MSG); die(); } }


#endif
