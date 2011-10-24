#pragma once

#include <queue.h>
#include <util.h>

typedef struct _tag_priorityq {
	int num_priorities;
	int state;
	queue *q[];
} priorityq;

priorityq *priorityq_new(int size, int num_priorities);

extern inline int priorityq_empty(priorityq *this) {
	return this->state == 0;
}

extern inline void* priorityq_pop(priorityq *this) {
	ASSERT(this->state, "empty");
	uint p = log2(this->state);
	queue *q = this->q[p];
	ASSERT(!queue_empty(q), "child queue empty! priority: %d, state: %x", p, this->state);
	void *rv = queue_pop(q);
	if (queue_empty(q)) this->state &= ~(1 << p);
	return rv;
}

extern inline void priorityq_push(priorityq *this, void* item, uint priority) {
	this->state |= 1 << priority;
	queue_push(this->q[priority], item);
}
