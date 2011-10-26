#include <buffer.h>
#include <memory.h>

buffer *buffer_new(uint size, uint elem_bytes) {
	ASSERT((elem_bytes & 0x3) == 0, "elem_size not aligned");
	int elem_size = NEXTHIGHESTWORD(elem_bytes);
	// extra 1 is an implementation detail needed it to diff empty and full states
	buffer *this = qmalloc(sizeof(buffer) + WORDS2BYTES(elem_size) * (size + 1));
	ASSERT(this, "out of memory");
	this->elem_size = elem_size;
	this->head = this->min;
	this->tail = this->min;
	this->max = this->min + size * elem_size;
	return this;
}
