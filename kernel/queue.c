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

inline void** queue_increment(queue *this, void** p) {
	return p == this->max ? this->min : p + 1;
}

inline int queue_empty(queue *this) {
	return this->head == this->tail;
}

inline int queue_full(queue *this) {
	return  this->head == queue_increment(this, this->tail);
}

inline void queue_push(queue *this, void* item) {
	ASSERT(!queue_full(this), "full");
	*this->tail = item;
	this->tail = queue_increment(this, this->tail);
}

inline void* queue_pop(queue *this) {
	ASSERT(!queue_empty(this), "empty");
	void* rv = *this->head;
	this->head = queue_increment(this, this->head);
	return rv;
}
