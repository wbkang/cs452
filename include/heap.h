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

static inline void* heap_peek(heap *this) {
	ASSERT(!heap_empty(this), "empty");
	return this->arr->data;
}

static inline int heap_peekkey(heap *this) {
	ASSERT(!heap_empty(this), "empty");
	return this->arr->key;
}

heap *heap_new(int size);
void heap_insert_min(heap *this, void* data, int key);
void *heap_extract_min(heap *this);
void heap_decrease_key(heap *this, int newkey, void* data);
// void heap_insert_max(heap *this, void* data, int key);
// void *heap_extract_max(heap *this);
