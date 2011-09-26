#pragma once

#include <util.h>

// ghetto templates, yo
#define STACK_NODE uint

typedef struct _tag_stack {
		STACK_NODE *top, *min, *max;
}*stack;

void stack_init(stack s, memptr address, uint size);

void stack_push(stack s, STACK_NODE p);

void stack_pushmany(stack s, STACK_NODE *p, uint len);

STACK_NODE stack_pop(stack s);

STACK_NODE *stack_popmany(stack s, uint len);

void stack_popmany2(stack s, uint len, STACK_NODE *dest);
