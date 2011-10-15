#pragma once

#include <util.h>

typedef struct _tag_stack {
	void** top;
	void** max;
	void* min[];
} stack;

stack *stack_new(uint size);
inline void stack_push(stack *this, void* item);
inline void* stack_pop(stack *this);
