#pragma once

#include <util.h>

typedef void* stack_node;

typedef struct _tag_stack {
		stack_node *top, *min, *max;
} stack;

void stack_init(stack *s, stack_node* address, uint size);

void stack_push(stack *s, stack_node p);

stack_node stack_pop(stack *s);

// void stack_pushmany(stack *s, stack_node *p, uint len);

// stack_node *stack_popmany(stack *s, uint len);

// void stack_popmany2(stack *s, uint len, stack_node *dest);
