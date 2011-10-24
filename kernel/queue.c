#include <queue.h>
#include <memory.h>

queue *queue_new(uint size) {
	// extra 1 is an implementation detail needed it to diff empty and full states
	queue *this = qmalloc(sizeof(queue) + sizeof(void*) * (size + 1));
	this->head = this->min;
	this->tail = this->min;
	this->max = this->min + size;
	return this;
}
