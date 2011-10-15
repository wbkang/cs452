#include <stack.h>
#include <util.h>
#include <memory.h>

stack *stack_new(uint size) {
	stack *this = (stack*) qmalloc(sizeof(stack) + sizeof(void*) * size);
	this->top = this->min;
	this->max = this->min + size;
	return this;
}

inline void stack_push(stack *this, void* item) {
	ASSERT(this->top != this->max, "full");
	*this->top = item;
	this->top++;
}

inline void* stack_pop(stack *this) {
	ASSERT(this->top != this->min, "empty");
	this->top--;
	return *this->top;
}
