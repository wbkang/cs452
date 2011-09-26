#include <stack.h>
#include <util.h>

// empty stack


void stack_init(stack *s, stack_node* address, uint size) {
	s->top = address;
	s->min = s->top;
	s->max = s->top + size;
}

void stack_push(stack *s, stack_node p) {
	ASSERT(s->top != s->max, "pushing a full stack");
	*s->top = p;
	s->top++;
}

void stack_pushmany(stack *s, stack_node *p, uint len) {
	for (; len != 0; p++, len--) {
		stack_push(s, *p);
	}
}

stack_node stack_pop(stack *s) {
	ASSERT(s->top != s->min, "popping an empty stack");
	s->top--;
	return *s->top;
}

stack_node *stack_popmany(stack *s, uint len) {
	ASSERT(s->top != s->min + len - 1, "popping more than on the stack");
	stack_node *rv = s->top - 1;
	s->top -= len;
	return rv;
}

void stack_popmany2(stack *s, uint len, stack_node *dest) {
	for (stack_node *lim = dest + len; dest < lim; dest++) {
		*dest = stack_pop(s);
	}
}
