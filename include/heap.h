#pragma once

#include <util.h>

typedef struct _tag_heap_item {
	int key;
	void* data;
} heap_item;

typedef struct _tag_heap {
	uint size;
	uint max_size;
	heap_item arr[];
} heap;

static inline int heap_empty(heap *this) {
	return this->size == 0;
}

static inline void heap_clear(heap *this) {
	this->size = 0;
}

static inline void* heap_peek(heap *this) {
	ASSERT(!heap_empty(this), "empty");
	return this->arr->data;
}

static inline int heap_peekkey(heap *this) {
	ASSERT(!heap_empty(this), "empty");
	return this->arr->key;
}

heap *heap_new(int size);
void heap_min_insert(heap *this, void* data, int key);
void *heap_min_extract(heap *this);
void heap_min_decrease_key(heap *this, void* data, int newkey);
// void heap_max_insert(heap *this, void* data, int key);
// void *heap_max_extract(heap *this);
