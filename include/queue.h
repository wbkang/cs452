#pragma once

#include <util.h>

typedef struct _tag_queue {
	void** head;
	void** tail;
	void** max;
	void* min[];
} queue;

queue *queue_new(uint size);
inline void** queue_increment(queue *this, void** p);
inline int queue_empty(queue *this);
inline int queue_full(queue *this);
inline void queue_push(queue *this, void* item);
inline void* queue_pop(queue *this);
