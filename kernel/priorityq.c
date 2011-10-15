#include <priorityq.h>
#include <memory.h>

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

inline int log2(uint v) {
	uint l, i = 0;
	if ((l = v >> 16)) { v = l; i |= 16; }
	if ((l = v >>  8)) { v = l; i |=  8; }
	if ((l = v >>  4)) { v = l; i |=  4; }
	if ((l = v >>  2)) { v = l; i |=  2; }
	if ((l = v >>  1)) { v = l; i |=  1; }
	return i;
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
