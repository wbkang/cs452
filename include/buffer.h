#pragma once

#include <util.h>
#include <string.h>

typedef struct _tag_buffer {
	int item_size;
	void** head;
	void** tail;
	void** max;
	void* min[];
} buffer;

buffer *buffer_new(uint size, uint elem_bytes);

static inline void** add_bytes(void** p, int bytes) {
	uint addr = (uint) p;
	uint rv = addr + bytes;
	return (void**) rv;
}

static inline void** buffer_increment(buffer *this, void** p) {
	return p == this->max ? this->min : add_bytes(p, this->item_size);
}

static inline int buffer_empty(buffer *this) {
	return this->head == this->tail;
}

static inline int buffer_full(buffer *this) {
	return this->head == buffer_increment(this, this->tail);
}

static inline void buffer_put(buffer *this, void const* item) {
	ASSERT(!buffer_full(this), "full");
	memcpy(this->tail, item, this->item_size);
	this->tail = buffer_increment(this, this->tail);
}

static inline void buffer_get(buffer *this, void* rv) {
	ASSERT(!buffer_empty(this), "empty");
	memcpy(rv, this->head, this->item_size);
	this->head = buffer_increment(this, this->head);
}
