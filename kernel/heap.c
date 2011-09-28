#include <heap.h>
#include <memory.h>

#define HEAP_NODEMAG(n) ((n).key)
#define HEAP_GT(h, i, j) (HEAP_NODEMAG(h->arr[i]) > HEAP_NODEMAG(h->arr[j]))
#define HEAP_GE(h, i, j) (HEAP_NODEMAG(h->arr[i]) >= HEAP_NODEMAG(h->arr[j]))
#define HEAP_PARENT(i) ((i) >> 1) // floor(i / 2)
#define HEAP_LEFTCHILD(i) ((i) << 1) // 2 * i
#define HEAP_RIGHTCHILD(i) (((i) << 1) | 1) // 2 * i + 1
#define HEAP_EXCHANGE(h, i, j) { \
	heap_item tmp = h->arr[i]; \
	h->arr[i] = h->arr[j]; \
	h->arr[j] = tmp; \
}

heap *heap_new(int size) {
	heap *h = kmalloc(sizeof(heap) + sizeof(heap_item) * size);
	h->size = 0;
	h->max_size = size;
	return h;
}

void heap_max_heapify(heap *h, uint i) {
	uint left = HEAP_LEFTCHILD(i);
	uint right = HEAP_RIGHTCHILD(i);
	uint largest;
	if (left < h->size && HEAP_GT(h, left, i)) {
		largest = left;
	} else {
		largest = i;
	}
	if (right < h->size && HEAP_GT(h, right, largest)) {
		largest = right;
	}
	if (largest != i) {
		HEAP_EXCHANGE(h, largest, i);
		heap_max_heapify(h, largest);
	}
}

void heap_max_heapify2(heap *h, uint i) {
	uint left, right, largest;
	for (;;) {
		left = HEAP_LEFTCHILD(i);
		if (left >= h->size) return;
		right = HEAP_RIGHTCHILD(i);
		if (right < h->size && HEAP_GT(h, right, i)) {
			largest = HEAP_GT(h, left, right) ? left : right;
		} else {
			if (HEAP_GT(h, left, i)) {
				largest = left;
			} else {
				return;
			}
		}
		HEAP_EXCHANGE(h, largest, i);
		i = largest;
	}
}

void* heap_extract_max(heap *h) {
	ASSERT(h->size != 0, "popping an empty heap");
	void* rv = h->arr[0].data;
	h->size--;
	h->arr[0] = h->arr[h->size];
	heap_max_heapify(h, 0);
	return rv;
}

void heap_max_insert(heap *h, void* data, int key) {
	ASSERT(h->size != h->max_size, "inserting into a full heap");
	uint i = h->size, parent;
	h->arr[i].data = data;
	h->arr[i].key = key;
	h->size++;
	while (i > 0) {
		parent = HEAP_PARENT(i);
		if (HEAP_GE(h, parent, i)) return;
		HEAP_EXCHANGE(h, i, parent);
		i = parent;
	}
}
