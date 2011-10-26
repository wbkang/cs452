#pragma once

#include <queue.h>
#include <util.h>

typedef struct _tag_priorityq {
	int num_priorities;
	int state;
	queue *q[];
} priorityq;

priorityq *priorityq_new(int size, int num_priorities);

static inline int priorityq_empty(priorityq *this) {
	return this->state == 0;
}

static inline void* priorityq_get(priorityq *this) {
	ASSERT(this->state, "empty");
	uint p = log2(this->state);
	queue *q = this->q[p];
	ASSERT(!queue_empty(q), "subqueue empty. priority: %d, state: %b", p, this->state);
	void *rv = queue_get(q);
	if (queue_empty(q)) {
		this->state &= ~(1 << p);
	}
	return rv;
}

static inline void priorityq_put(priorityq *this, void* item, uint priority) {
	this->state |= 1 << priority;
	queue_put(this->q[priority], item);
}
