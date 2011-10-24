#pragma once

#include <util.h>
#include <string.h>

typedef struct _tag_buffer {
	int elem_size;
	void** head;
	void** tail;
	void** max;
	void* min[];
} buffer;

buffer *buffer_new(uint size, uint elem_bytes);

static inline void** buffer_increment(buffer *this, void** p) {
	return p == this->max ? this->min : p + this->elem_size;
}

static inline int buffer_empty(buffer *this) {
	return this->head == this->tail;
}

static inline int buffer_full(buffer *this) {
	return this->head == buffer_increment(this, this->tail);
}

static inline void buffer_put(buffer *this, void* item) {
	ASSERT(!buffer_full(this), "full");
	memcpy(this->tail, item, WORDS2BYTES(this->elem_size));
	this->tail = buffer_increment(this, this->tail);
}

static inline void buffer_get(buffer *this, void* rv) {
	ASSERT(!buffer_empty(this), "empty");
	memcpy(rv, this->head, WORDS2BYTES(this->elem_size));
	this->head = buffer_increment(this, this->head);
}
