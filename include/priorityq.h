#pragma once

#include <queue.h>
#include <util.h>

typedef struct _tag_priorityq {
	int num_priorities;
	int state;
	queue *q[];
} priorityq;

priorityq *priorityq_new(int size, int num_priorities);
inline int priorityq_empty(priorityq *this);
inline void* priorityq_pop(priorityq *this);
inline void priorityq_push(priorityq *this, void* item, uint priority);
