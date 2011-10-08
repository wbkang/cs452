#include <heap.h>
#include <memory.h>

heap *heap_new(int size) {
	heap *h = qmalloc(sizeof(heap) + sizeof(heap_item) * size);
	h->size = 0;
	h->max_size = size;
	return h;
}

inline heap_item *heap_peek(heap *h) {
	return h->size == 0 ? NULL : h->arr;
}

inline uint heap_parent(uint i) {
	return i >> 1; // i / 2
}

inline uint heap_leftchild(uint i) {
	return i << 1; // i * 2
}

inline uint heap_rightchild(uint i) {
	return (i << 1) | 1; // i * 2 + 1
}

inline uint heap_swap(heap *h, uint i, uint j) {
	heap_item tmp = h->arr[i];
	h->arr[i] = h->arr[j];
	h->arr[j] = tmp;
	return j;
}

inline int heap_put(heap *h, void *data, int key) {
	ASSERT(h->size < h->max_size, "full");
	uint i = h->size;
	h->arr[i].data = data;
	h->arr[i].key = key;
	h->size++;
	return i;
}

inline void heap_bubbleup_min(heap *h, uint i) {
	while (i > 0) {
		uint p = heap_parent(i);
		if (h->arr[p].key <= h->arr[i].key) return;
		i = heap_swap(h, i, p);
	}
}

inline void heap_bubbleup_max(heap *h, uint i) {
	while (i > 0) {
		uint p = heap_parent(i);
		if (h->arr[p].key >= h->arr[i].key) return;
		i = heap_swap(h, i, p);
	}
}

void heap_insert_min(heap *h, void* data, int key) {
	heap_bubbleup_min(h, heap_put(h, data, key));
}

void heap_insert_max(heap *h, void* data, int key) {
	heap_bubbleup_max(h, heap_put(h, data, key));
}

inline void heap_heapify_min(heap *h, uint i) {
	for (;;) {
		uint left = heap_leftchild(i);
		if (left >= h->size) return; // no left child => no right child
		uint right = heap_rightchild(i);
		if (right < h->size && h->arr[right].key < h->arr[i].key) {
			if (h->arr[left].key < h->arr[right].key) {
				i = heap_swap(h, i, left);
			} else {
				i = heap_swap(h, i, right);
			}
		} else {
			if (h->arr[left].key < h->arr[i].key) {
				i = heap_swap(h, i, left);
			} else {
				return;
			}
		}
	}
}

inline void heap_heapify_max(heap *h, uint i) {
	for (;;) {
		uint left = heap_leftchild(i);
		if (left >= h->size) return; // no left child => no right child
		uint right = heap_rightchild(i);
		if (right < h->size && h->arr[right].key > h->arr[i].key) {
			if (h->arr[left].key > h->arr[right].key) {
				i = heap_swap(h, i, left);
			} else {
				i = heap_swap(h, i, right);
			}
		} else {
			if (h->arr[left].key > h->arr[i].key) {
				i = heap_swap(h, i, left);
			} else {
				return;
			}
		}
	}
}

void* heap_extract_min(heap *h) {
	heap_item *top = heap_peek(h);
	ASSERT(top, "empty");
	h->size--;
	void* rv = top->data;
	if (h->size == 0) {
		*top = h->arr[h->size];
		heap_heapify_min(h, 0);
	}
	return rv;
}

void* heap_extract_max(heap *h) {
	heap_item *top = heap_peek(h);
	ASSERT(top, "empty");
	h->size--;
	void* rv = top->data;
	if (h->size > 0) {
		*top = h->arr[h->size];
		heap_heapify_max(h, 0);
	}
	return rv;
}
