#pragma once

#include <util.h>
#include <queue.h>

typedef struct _tag_priorityq {
	int num_priorities;
	int len;
	queue **q;
} priorityq;

#define PRIORITYQ_EMPTY(pq) ((pq)->len == 0)

void priorityq_init(priorityq *pq, int num_priorities, int size);

int priorityq_maxp(priorityq *pq);

void *priorityq_pop(priorityq *pq);

void priorityq_push(priorityq *pq, void* item, uint priority);
