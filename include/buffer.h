#pragma once

#include <util.h>
#include <string.h>

typedef struct _tag_buffer {
	int item_size;
	void* head;
	void* tail;
	void* max;
	void* min[];
} buffer;

buffer *buffer_new(uint size, uint elem_bytes);

static inline void* buffer_inc(void* p, int bytes) {
	return (void*) ((uint) p + bytes);
}

static inline void* buffer_increment(buffer *this, void* p) {
	return p == this->max ? this->min : buffer_inc(p, this->item_size);
}

static inline int buffer_empty(buffer *this) {
	return this->head == this->tail;
}

static inline int buffer_full(buffer *this) {
	return this->head == buffer_increment(this, this->tail);
}

static inline void buffer_put(buffer *this, void const* item, int item_size) {
	ASSERT(!buffer_full(this), "full");
	ASSERT(item_size <= this->item_size, "item size too big %d", item_size);
	memcpy(this->tail, item, item_size);
	this->tail = buffer_increment(this, this->tail);
}

static inline void buffer_get(buffer *this, void* rv) {
	ASSERT(!buffer_empty(this), "empty");
	memcpy(rv, this->head, this->item_size);
	this->head = buffer_increment(this, this->head);
}

static inline int buffer_size(buffer *this) {
	int count = 0;
	for (void *p = this->head; p != this->tail; p = buffer_increment(this, p)) {
		count++;
	}
	return count;
}
