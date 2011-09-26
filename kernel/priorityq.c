#include <priorityq.h>

void priorityq_init(priorityq *pq, int num_priorities, int size) {
	pq->num_priorities = num_priorities;
	pq->len = 0;
	for (int i = 0; i < size; ++i) {
		queue_init(pq->q[i], size);
	}
}

int priorityq_maxp(priorityq *pq) { // index of non-empty q with max priority
	int i = pq->num_priorities - 1;
	while (QUEUE_EMPTY(pq->q[i])) {
		--i;
	}
	return i;
}

void *priorityq_pop(priorityq *pq) {
	ASSERT(!PRIORITYQ_EMPTY(pq), "popping an empty priority queue");
	pq->len--;
	return queue_pop(pq->q[priorityq_maxp(pq)]);
}

void priorityq_push(priorityq *pq, void* item, uint priority) {
	ASSERT(priority < pq->num_priorities, "priority too high");
	pq->len++;
	queue_push(pq->q[priority], item);
}
