#include <priorityq.h>
#include <memory.h>

priorityq *priorityq_new(int size, int num_priorities) {
	ASSERT(num_priorities <= 32, "too many priorities, bithack requires <= 32");
	priorityq *pq = qmalloc(sizeof(priorityq) + sizeof(queue*) * num_priorities);
	pq->num_priorities = num_priorities;
	pq->state = 0;
	for (int i = 0; i < num_priorities; i++) pq->q[i] = queue_new(size);
	return pq;
}

inline int priorityq_empty(priorityq *pq) {
	return pq->state == 0;
}

inline int log2(register unsigned int v) {
	register unsigned int l, i = 0;
	if ((l = v >> 16)) { v = l; i |= 16; }
	if ((l = v >> 8))  { v = l; i |= 8;  }
	if ((l = v >> 4))  { v = l; i |= 4;  }
	if ((l = v >> 2))  { v = l; i |= 2;  }
	if ((l = v >> 1))  { v = l; i |= 1;  }
	return i;
}

inline void* priorityq_pop(priorityq *pq) {
	ASSERT(pq->state, "priorityq is empty");
	uint p = log2(pq->state);
	queue *q = pq->q[p];
	ASSERT(!queue_empty(q), "queue is actually empty! priority: %d, state: %x", p, pq->state);
	void *rv = queue_pop(q);
	if (queue_empty(q)) pq->state &= ~(1 << p);
	return rv;
}

inline void priorityq_push(priorityq *pq, void* item, uint priority) {
	pq->state |= 1 << priority;
	queue_push(pq->q[priority], item);
}

