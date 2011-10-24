#pragma once

#include <util.h>

typedef struct _tag_stack {
	void** top;
	void** max;
	void* min[];
} stack;

stack *stack_new(uint size);

static inline void stack_push(stack *this, void* item) {
	ASSERT(this->top != this->max, "full");
	*this->top = item;
	this->top++;
}

static inline void* stack_pop(stack *this) {
	ASSERT(this->top != this->min, "empty");
	this->top--;
	return *this->top;
}
