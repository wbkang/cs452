#pragma once

#include <queue.h>
#include <util.h>

typedef struct _tag_priorityq {
		int num_priorities;
		int len;
		queue *q[]; // this needs to point to an array of queue pointers
} priorityq;

#define PRIORITYQ_EMPTY(pq) ((pq)->len == 0)

priorityq *priorityq_new(int size, int num_priorities, memptr *heap);

int priorityq_maxp(priorityq *pq);

void *priorityq_pop(priorityq *pq);

void priorityq_push(priorityq *pq, void* item, uint priority);
