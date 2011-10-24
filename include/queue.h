#pragma once

#include <util.h>

typedef struct _tag_queue {
	void** head;
	void** tail;
	void** max;
	void* min[];
} queue;

queue *queue_new(uint size);

extern inline void** queue_increment(queue *this, void** p) {
	return p == this->max ? this->min : p + 1;
}

extern inline int queue_empty(queue *this) {
	return this->head == this->tail;
}

extern inline int queue_full(queue *this) {
	return this->head == queue_increment(this, this->tail);
}

extern inline void queue_push(queue *this, void* item) {
	ASSERT(!queue_full(this), "full");
	*this->tail = item;
	this->tail = queue_increment(this, this->tail);
}

extern inline void* queue_pop(queue *this) {
	ASSERT(!queue_empty(this), "empty");
	void* rv = *this->head;
	this->head = queue_increment(this, this->head);
	return rv;
}
