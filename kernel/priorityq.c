#include <priorityq.h>
#include <memory.h>
#include <util.h>

priorityq *priorityq_new(int size, int num_priorities) {
	ASSERT(num_priorities <= 32, "too many priorities, bithack requires <= 32");
	priorityq *this = qmalloc(sizeof(priorityq) + sizeof(queue*) * num_priorities);
	this->num_priorities = num_priorities;
	this->state = 0;
	for (int i = 0; i < num_priorities; i++) {
		this->q[i] = queue_new(size);
	}
	return this;
}

inline int priorityq_empty(priorityq *this) {
	return this->state == 0;
}

inline void* priorityq_pop(priorityq *this) {
	ASSERT(this->state, "empty");
	uint p = log2(this->state);
	queue *q = this->q[p];
	ASSERT(!queue_empty(q), "child queue empty! priority: %d, state: %x", p, this->state);
	void *rv = queue_pop(q);
	if (queue_empty(q)) this->state &= ~(1 << p);
	return rv;
}

inline void priorityq_push(priorityq *this, void* item, uint priority) {
	this->state |= 1 << priority;
	queue_push(this->q[priority], item);
}
