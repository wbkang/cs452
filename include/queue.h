#pragma once

#include <util.h>

typedef struct _tag_queue {
		uint head;
		uint tail;
		uint size;
		void* arr[];
} queue;

#define QUEUE_MEMSIZE(sz) (sizeof(queue)+ (sz))

#define QUEUE_EMPTY(q) ((q)->head == (q)->tail)

#define QUEUE_INCREMENT(q, i) (((i) + 1 < (q)->size) ? (i) + 1 : 0)

#define QUEUE_FULL(q) (QUEUE_INCREMENT((q), (q)->tail) == (q)->head)

queue* queue_new(uint size, memptr *heap);

void queue_push(queue *q, void* item);

void *queue_pop(queue *q);
