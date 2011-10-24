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

heap *heap_new(int size);
inline heap_item *heap_peek(heap *this);
void heap_insert_min(heap *this, void* data, int key);
void *heap_extract_min(heap *this);
// void heap_insert_max(heap *this, void* data, int key);
// void *heap_extract_max(heap *this);
