#include <stack.h>
#include <util.h>
#include <memory.h>

#include <hardware.h>
#include <rawio.h>

stack *stack_new(uint size) {
	stack *s = (stack*) qmalloc(sizeof(stack) + sizeof(void*) * size);
	s->top = s->arr;
	s->min = s->top;
	s->max = s->top + size;
	return s;
}

void stack_push(stack *s, void* p) {
	ASSERT(s->top != s->max, "pushing a full stack");
	*s->top = p;
	s->top++;
}

void* stack_pop(stack *s) {
	ASSERT(s->top != s->min, "popping an empty stack");
	s->top--;
	return *s->top;
}
