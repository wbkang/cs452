#include <queue.h>

queue* queue_init(uint size, memptr *heap) {
	ASSERT(size > 0, "queue size needs to be greater than 0");
	// allocate memory
	queue *q = (queue*) *heap;
	*heap += sizeof(queue) + sizeof(void*) * (size + 1); // extra 1 is an implementation detail, need it to diff. empty and full states
	// initialize
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
