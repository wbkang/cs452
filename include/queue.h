#pragma once

#include <util.h>

typedef struct _tag_queue {
	void** head;
	void** tail;
	void** max;
	void* min[];
} queue;

queue *queue_new(uint size);

inline void** queue_increment(queue *q, void** p);

inline int queue_empty(queue *q);

inline int queue_full(queue *q);

inline void queue_push(queue *q, void* item);

inline void* queue_pop(queue *q);
