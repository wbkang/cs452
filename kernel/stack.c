#include <stack.h>
#include <util.h>

void stack_init(stack s, memptr address, uint size) {
	s->top = (STACK_NODE *) address;
	s->min = s->top;
	s->max = s->top + size;
}

void stack_push(stack s, STACK_NODE p) {
	ASSERT(s->top != s->max, "pushing a full stack");
	*s->top = p;
	s->top++;
}

void stack_pushmany(stack s, STACK_NODE *p, uint len) {
	for (; len != 0; p++, len--) {
		stack_push(s, *p);
	}
}

STACK_NODE stack_pop(stack s) {
	ASSERT(s->top != s->min, "popping an empty stack");
	s->top--;
	return *s->top;
}

STACK_NODE *stack_popmany(stack s, uint len) {
	ASSERT(s->top != s->min + len - 1, "popping more than on the stack");
	STACK_NODE *rv = s->top - 1;
	s->top -= len;
	return rv;
}

void stack_popmany2(stack s, uint len, STACK_NODE *dest) {
	for (STACK_NODE *lim = dest + len; dest < lim; dest++) {
		*dest = stack_pop(s);
	}
}
