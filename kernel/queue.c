#include <queue.h>
#include <memory.h>

queue* queue_new(uint size) {
	ASSERT(size > 0, "queue size needs to be greater than 0");
	// extra 1 is an implementation detail, need it to diff.
	// empty and full states
	queue *q = kmalloc(sizeof(queue) + sizeof(void*) * (size + 1));
	q->head = 0;
	q->tail = 0;
	q->size = size + 1;
	return q;
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
