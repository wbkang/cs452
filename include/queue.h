#pragma once;

#include <util.h>

typedef struct _tag_queue {
		uint head;
		uint tail;
		uint size;
		void** arr;
} queue;

#define QUEUE_EMPTY(q) ((q)->head == (q)->tail)

#define QUEUE_FULL(q) (QUEUE_INCREMENT((q), (q)->tail) == (q)->head)

#define QUEUE_INCREMENT(q, i) (((i) + 1 < (q)->size) ? (i) + 1 : 0)

void queue_init(queue *q, uint size);

void queue_push(queue *q, void* item);

void *queue_pop(queue *q);
