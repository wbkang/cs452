#include <queue.h>
#include <memory.h>

queue *queue_new(uint size) {
	ASSERT(size > 0, "queue size needs to be greater than 0");
	// extra 1 is an implementation detail needed it to diff empty and full states
	queue *q = qmalloc(sizeof(queue) + sizeof(void*) * (size + 1));
	q->head = q->min;
	q->tail = q->min;
	q->max = q->min + size;
	return q;
}

inline void** queue_increment(queue *q, void** p) {
	return p == q->max ? q->min : p + 1;
}

inline int queue_empty(queue *q) {
	return q->head == q->tail;
}

inline int queue_full(queue *q) {
	return queue_increment(q, q->tail) == q->head;
}

inline void queue_push(queue *q, void* item) {
	ASSERT(!queue_full(q), "full");
	*q->tail = item;
	q->tail = queue_increment(q, q->tail);
}

inline void* queue_pop(queue *q) {
	ASSERT(!queue_empty(q), "empty");
	void* rv = *q->head;
	q->head = queue_increment(q, q->head);
	return rv;
}
