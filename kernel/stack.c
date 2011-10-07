#include <stack.h>
#include <util.h>
#include <memory.h>

stack *stack_new(uint size) {
	stack *s = (stack*) qmalloc(sizeof(stack) + sizeof(void*) * size);
	s->top = s->min;
	s->max = s->min + size;
	return s;
}

inline void stack_push(stack *s, void* item) {
	ASSERT(s->top != s->max, "pushing a full stack");
	*s->top = item;
	s->top++;
}

inline void* stack_pop(stack *s) {
	ASSERT(s->top != s->min, "popping an empty stack");
	s->top--;
	return *s->top;
}
