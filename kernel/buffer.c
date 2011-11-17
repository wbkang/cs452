#include <buffer.h>
#include <memory.h>

buffer *buffer_new(uint size, uint item_size) {
	size += 1; // implementation detail to diff empty and full states
	buffer *this = qmalloc(sizeof(buffer) + item_size * size);
	ASSERT(this, "out of memory");
	this->item_size = item_size;
	this->head = this->min;
	this->tail = this->min;
	this->max = buffer_inc(this->min, (size - 1) * item_size);
	return this;
}
