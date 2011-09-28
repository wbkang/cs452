#pragma once

#include <util.h>

typedef struct _tag_heap_item {
		void* data;
		int key;
} heap_item;

typedef struct _tag_heap {
		uint size;
		uint max_size;
		heap_item arr[];
} heap;

heap *heap_new(int size, memptr *heap);

void heap_max_heapify(heap *h, uint i);

void heap_max_heapify2(heap *h, uint i);

void *heap_extract_max(heap *h);

void heap_max_insert(heap *h, void* data, int key);
