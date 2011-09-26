#pragma once

#include <util.h>

typedef struct _tag_queue {
		uint head;
		uint tail;
		uint size;
		uint count;
		void* arr[]; // this needs to have the length of size
} queue;

#define QUEUE_MEMSIZE(sz) (sizeof(queue)+ (sz))

#define QUEUE_EMPTY(q) ((q)->count == 0)

#define QUEUE_FULL(q) ((q)->count == (q)->size)

void queue_init(queue *q, uint size);

void queue_push(queue *q, void* item);

void *queue_pop(queue *q);
