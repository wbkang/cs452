#include <priorityq.h>
#include <memory.h>

#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n

static const char LogTable256[256] = {
    -1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
    LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};

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

int log2pp(unsigned int v) {
	register unsigned int a, b;
	if ((a = v >> 16)) {
		if ((b = a >> 8)) {
			return 24 + LogTable256[b];
		} else {
			return 16 + LogTable256[a];
		}
	} else {
		if ((b = v >> 8)) {
			return 8 + LogTable256[b];
		} else {
			return LogTable256[v];
		}
	}
}

inline int log2(register unsigned int v) {
	register unsigned int i = 0;
	if (v & 0xffff0000) { v >>= 16; i |= 16; }
	if (v & 0xff00)     { v >>= 8;  i |= 8;  }
	if (v & 0xf0)       { v >>= 4;  i |= 4;  }
	if (v & 0xc)        { v >>= 2;  i |= 2;  }
	if (v & 0x2)        { v >>= 1;  i |= 1;  }
	return i;
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

inline int priorityq_maxpp(priorityq *pq) {
	return log2(pq->len);
}

inline int priorityq_maxppp(priorityq *pq) {
	unsigned int v = pq->len;  // 32-bit value to find the log2 of
	const unsigned int b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
	const unsigned int S[] = {1, 2, 4, 8, 16};
	register unsigned int r = 0;
	for (int i = 4; i >= 0; i--) {
		if (v & b[i]) {
			v >>= S[i];
			r |= S[i];
		}
	}
	return r;
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

inline void* priorityq_pop2(priorityq *pq) {
	uint p = priorityq_maxp(pq);
	queue *q = pq->q[p];
	if (queue_empty(q)) pq->len |= ~(1 << p);
	return queue_pop(q);
}

inline void priorityq_push2(priorityq *pq, void* item, uint priority) {
	pq->len |= 1 << priority;
	queue_push(pq->q[priority], item);
}
