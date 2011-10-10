#pragma once

#include <queue.h>
#include <util.h>

typedef struct _tag_priorityq {
	int num_priorities;
	int state;
	queue *q[];
} priorityq;

priorityq *priorityq_new(int size, int num_priorities);

inline int priorityq_empty(priorityq *pq);

inline void* priorityq_pop(priorityq *pq);

inline void priorityq_push(priorityq *pq, void* item, uint priority);
