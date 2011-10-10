#include <priorityq.h>
#include <memory.h>
inline static nlz(register unsigned int x);

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

inline int priorityq_maxp(priorityq *pq) {
	int p;
	for (p = pq->num_priorities - 1; queue_empty(pq->q[p]); --p);
	return p;
}

inline void* priorityq_pop(priorityq *pq) {
	ASSERT(!priorityq_empty(pq), "popping an empty priority queue");
	pq->len--;
	return queue_pop(pq->q[priorityq_maxp(pq)]);
}

inline void priorityq_push(priorityq *pq, void* item, uint priority) {
	ASSERT(priority < pq->num_priorities, "priority too high (%x)", priority);
	pq->len++;
	queue_push(pq->q[priority], item);
}

// from hacker's delight
inline static nlz(register unsigned int x) {
	register int y, m, n;

	y = -(x >> 16);
	m = (y >> 16) & 16;
	n = 16 - m;
	x = x >> m;

	y = x - 0x100;
	m = (y >> 16) & 8;
	n = n + m;
	x = x << m;

	y = x - 0x1000;
	m = (y >> 16) & 4;
	n = n + m;
	x = x << m;

	y = x - 0x4000;
	m = (y >> 16) & 2;
	n = n + m;
	x = x << m;

	y = x >> 14;
	m = y & ~(y >> 1);
	return n + 2 - m;
}
