#pragma once

#include <util.h>

typedef struct _tag_stack {
		void** top;
		void** min;
		void** max;
		void* arr[];
} stack;

stack *stack_new(uint size);

void stack_push(stack *s, void *item);

void* stack_pop(stack *s);
