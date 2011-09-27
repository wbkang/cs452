#include <stack.h>
#include <util.h>

stack *new_stack(uint size, memptr *heap) {
	// allocate memory
	stack *s = (stack*) *heap;
	*heap += sizeof(stack) + sizeof(void*) * size;
	// initialize
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
