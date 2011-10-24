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
