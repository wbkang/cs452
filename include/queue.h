#pragma once

#include <util.h>

typedef struct _tag_queue {
	void** head;
	void** tail;
	void** max;
	void* min[];
} queue;

queue *queue_new(uint size);

static inline void** queue_increment(queue *this, void** p) {
	return p == this->max ? this->min : p + 1;
}

static inline int queue_empty(queue *this) {
	return this->head == this->tail;
}

static inline int queue_full(queue *this) {
	return this->head == queue_increment(this, this->tail);
}

static inline void queue_put(queue *this, void* item) {
	ASSERT(!queue_full(this), "full");
	*this->tail = item;
	this->tail = queue_increment(this, this->tail);
}

static inline void* queue_get(queue *this) {
	ASSERT(!queue_empty(this), "empty");
	void* rv = *this->head;
	this->head = queue_increment(this, this->head);
	return rv;
}

static inline void* queue_peek(queue *this) {
	ASSERT(!queue_empty(this), "empty");
	return *this->head;
}
