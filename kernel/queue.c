#include <queue.h>
#include <memory.h>

queue *queue_new(uint size) {
	size += 1; // implementation detail to diff empty and full states
	queue *this = qmalloc(sizeof(queue) + sizeof(void*) * size);
	this->head = this->min;
	this->tail = this->min;
	this->max = this->min + size - 1;
	return this;
}
