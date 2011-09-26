#include <queue.h>

#define QUEUE_INCREMENT(q, i) (((i) + 1 < (q)->size) ? (i) + 1 : 0)

void queue_init(queue *q, uint size) {
	ASSERT(q, "not a valid queue space");
	ASSERT(size > 0, "queue size needs to be greater than 0");
	q->head = 0;
	q->tail = 0;
	q->size = size;
	q->count = 0;
}

void queue_push(queue *q, void* item) {
	ASSERT(!QUEUE_FULL(q), "pushing a full queue");
	q->arr[q->tail] = item;
	q->tail = QUEUE_INCREMENT(q, q->tail);
	q->count++;
}

void *queue_pop(queue *q) {
	ASSERT(!QUEUE_EMPTY(q), "popping an empty queue");
	void *item = q->arr[q->head];
	q->head = QUEUE_INCREMENT(q, q->head);
	q->count--;
	return item;
}
