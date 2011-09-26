#include <queue.h>

void queue_init(queue *q, uint size) {
	q->head = 0;
	q->tail = 0;
	q->size = size;
}

void queue_push(queue *q, void* item) {
	ASSERT(!QUEUE_FULL(q), "pushing a full queue");
	q->arr[q->tail] = item;
	q->tail = QUEUE_INCREMENT(q, q->tail);
}

void *queue_pop(queue *q) {
	ASSERT(!QUEUE_EMPTY(q), "popping an empty queue");
	void *item = q->arr[q->head];
	q->head = QUEUE_INCREMENT(q, q->head);
	return item;
}
