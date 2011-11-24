#include <heap.h>
#include <memory.h>

heap *heap_new(int size) {
	heap *this = qmalloc(sizeof(heap) + sizeof(heap_item) * size);
	ASSERT(this, "out of memory");
	this->size = 0;
	this->max_size = size;
	return this;
}

static inline uint heap_parent(uint i) {
	return i >> 1; // i / 2
}

static inline uint heap_leftchild(uint i) {
	return i << 1; // i * 2
}

static inline uint heap_rightchild(uint i) {
	return (i << 1) | 1; // i * 2 + 1
}

static inline uint heap_swap(heap *this, uint i, uint j) {
	heap_item tmp = this->arr[i];
	this->arr[i] = this->arr[j];
	this->arr[j] = tmp;
	return j;
}

static inline int heap_put(heap *this, void *data, int key) {
	ASSERT(this->size < this->max_size, "full");
	uint i = this->size;
	this->arr[i] = (heap_item) {.key = key, .data = data};
	this->size++;
	return i;
}

static inline void heap_min_bubbleup(heap *this, uint i) {
	while (i > 0) {
		uint p = heap_parent(i);
		if (this->arr[p].key <= this->arr[i].key) return;
		i = heap_swap(this, i, p);
	}
}

void heap_min_decrease_key(heap *this, void* data, int newkey) {
	int pos = -1;
	for (int i = 0; i < this->size; i++) {
		if (this->arr[i].data == data) {
			pos = i;
		}
	}
	ASSERT(pos != -1, "heap: %x, data %x not found. newkey: %d", this, data, newkey);

	heap_item *item = &this->arr[pos];
	ASSERT(item->key >= newkey, "newkey %d is not smaller than the original key %d", newkey, item->key);

	item->key = newkey;
	heap_min_bubbleup(this, pos);
}
// inline void heap_max_bubbleup(heap *this, uint i) {
// 	while (i > 0) {
// 		uint p = heap_parent(i);
// 		if (this->arr[p].key >= this->arr[i].key) return;
// 		i = heap_swap(this, i, p);
// 	}
// }

void heap_min_insert(heap *this, void* data, int key) {
	heap_min_bubbleup(this, heap_put(this, data, key));
}

// void heap_max_insert(heap *this, void* data, int key) {
// 	heap_max_bubbleup(this, heap_put(this, data, key));
// }

static inline void heap_min_heapify(heap *this, uint i) {
	for (;;) {
		uint left = heap_leftchild(i);
		if (left >= this->size) return; // no left child -> no right child
		uint right = heap_rightchild(i);
		if (right < this->size && this->arr[right].key < this->arr[i].key) {
			if (this->arr[left].key < this->arr[right].key) {
				i = heap_swap(this, i, left);
			} else {
				i = heap_swap(this, i, right);
			}
		} else if (this->arr[left].key < this->arr[i].key) {
			i = heap_swap(this, i, left);
		} else {
			return;
		}
	}
}

// inline void heap_max_heapify(heap *this, uint i) {
// 	for (;;) {
// 		uint left = heap_leftchild(i);
// 		if (left >= this->size) return; // no left child -> no right child
// 		uint right = heap_rightchild(i);
// 		if (right < this->size && this->arr[right].key > this->arr[i].key) {
// 			if (this->arr[left].key > this->arr[right].key) {
// 				i = heap_swap(this, i, left);
// 			} else {
// 				i = heap_swap(this, i, right);
// 			}
// 		} else if (this->arr[left].key > this->arr[i].key) {
// 			i = heap_swap(this, i, left);
// 		} else {
// 			return;
// 		}
// 	}
// }

void* heap_min_extract(heap *this) {
	ASSERT(!heap_empty(this), "empty");
	heap_item *top = this->arr;
	this->size--;
	void* rv = top->data;
	if (this->size > 0) {
		*top = this->arr[this->size];
		heap_min_heapify(this, 0);
	}
	return rv;
}

// void* heap_max_extract(heap *this) {
//	ASSERT(!heap_empty(this), "empty");
//	heap_item *top = this->arr;
// 	this->size--;
// 	void* rv = top->data;
// 	if (this->size > 0) {
// 		*top = this->arr[this->size];
// 		heap_max_heapify(this, 0);
// 	}
// 	return rv;
// }
