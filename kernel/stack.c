#include <stack.h>
#include <util.h>
#include <memory.h>

stack *stack_new(uint size) {
	stack *this = (stack*) qmalloc(sizeof(stack) + sizeof(void*) * size);
	this->top = this->min;
	this->max = this->min + size;
	return this;
}
