#include <heap.h>

void heap_max_heapify(heap h, uint i) {
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

void heap_max_heapify2(heap h, uint i) {
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

HEAP_NODE heap_extract_max(heap h) {
	ASSERT(h->size != 0, "popping an empty heap");
	HEAP_NODE max = HEAP_TOP(h);
	h->size--;
	HEAP_TOP(h) = h->arr[h->size];
	heap_max_heapify(h, 0);
	return max;
}

void heap_increase_key(heap h, uint i, HEAP_NODE key) {
	ASSERT(key >= HEAP_NODEMAG(h->arr[i]), "new key smaller than current key");
	h->arr[i] = key;
	uint parent;
	while (i > 0) {
		parent = HEAP_PARENT(i);
		if (HEAP_GE(h, parent, i)) return;
		HEAP_EXCHANGE( h, i, parent);
		i = parent;
	}
}

void heap_max_insert(heap h, HEAP_NODE key) {
	ASSERT(h->size != h->max_size, "inserting into a full heap");
	h->arr[h->size] = 0;
	heap_increase_key(h, h->size, key);
	h->size++;
}
