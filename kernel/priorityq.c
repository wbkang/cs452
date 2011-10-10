#include <priorityq.h>
#include <memory.h>

priorityq *priorityq_new(int size, int num_priorities) {
	priorityq *pq = qmalloc(sizeof(priorityq) + sizeof(queue*) * num_priorities);
	pq->num_priorities = num_priorities;
	pq->len = 0;
	for (int i = 0; i < num_priorities; i++) pq->q[i] = queue_new(size);
	return pq;
}

inline int priorityq_empty(priorityq *pq) {
	return pq->len == 0;
}

inline int log2p(register unsigned int v) {
	register unsigned int l, i = 0;
	if ((l = v >> 16)) { v = l; i |= 16; }
	if ((l = v >> 8))  { v = l; i |= 8;  }
	if ((l = v >> 4))  { v = l; i |= 4;  }
	if ((l = v >> 2))  { v = l; i |= 2;  }
	if ((l = v >> 1))  { v = l; i |= 1;  }
	return i;
}

inline void* priorityq_pop(priorityq *pq) {
	uint p = log2p(pq->len);
	queue *q = pq->q[p];
	ASSERT(!queue_empty(q), "queue is actually not empty! priority: %d, len: %x", p, pq->len);
	void *rv = queue_pop(q);
	if (queue_empty(q)) pq->len &= ~(1 << p);
	return rv;
}

inline void priorityq_push(priorityq *pq, void* item, uint priority) {
	pq->len |= 1 << priority;
	queue_push(pq->q[priority], item);
}

