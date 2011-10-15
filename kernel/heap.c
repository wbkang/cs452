#include <heap.h>
#include <memory.h>

heap *heap_new(int size) {
	heap *this = qmalloc(sizeof(heap) + sizeof(heap_item) * size);
	ASSERT(this, "out of memory");
	this->size = 0;
	this->max_size = size;
	return this;
}

inline heap_item *heap_peek(heap *this) {
	return this->size == 0 ? NULL : this->arr;
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

inline uint heap_swap(heap *this, uint i, uint j) {
	heap_item tmp = this->arr[i];
	this->arr[i] = this->arr[j];
	this->arr[j] = tmp;
	return j;
}

inline int heap_put(heap *this, void *data, int key) {
	ASSERT(this->size < this->max_size, "full");
	uint i = this->size;
	this->arr[i].data = data;
	this->arr[i].key = key;
	this->size++;
	return i;
}

inline void heap_bubbleup_min(heap *this, uint i) {
	while (i > 0) {
		uint p = heap_parent(i);
		if (this->arr[p].key <= this->arr[i].key) return;
		i = heap_swap(this, i, p);
	}
}

inline void heap_bubbleup_max(heap *this, uint i) {
	while (i > 0) {
		uint p = heap_parent(i);
		if (this->arr[p].key >= this->arr[i].key) return;
		i = heap_swap(this, i, p);
	}
}

void heap_insert_min(heap *this, void* data, int key) {
	heap_bubbleup_min(this, heap_put(this, data, key));
}

void heap_insert_max(heap *this, void* data, int key) {
	heap_bubbleup_max(this, heap_put(this, data, key));
}

inline void heap_heapify_min(heap *this, uint i) {
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

inline void heap_heapify_max(heap *this, uint i) {
	for (;;) {
		uint left = heap_leftchild(i);
		if (left >= this->size) return; // no left child -> no right child
		uint right = heap_rightchild(i);
		if (right < this->size && this->arr[right].key > this->arr[i].key) {
			if (this->arr[left].key > this->arr[right].key) {
				i = heap_swap(this, i, left);
			} else {
				i = heap_swap(this, i, right);
			}
		} else if (this->arr[left].key > this->arr[i].key) {
			i = heap_swap(this, i, left);
		} else {
			return;
		}
	}
}

void* heap_extract_min(heap *this) {
	heap_item *top = heap_peek(this);
	ASSERT(top, "empty");
	this->size--;
	void* rv = top->data;
	if (this->size > 0) {
		*top = this->arr[this->size];
		heap_heapify_min(this, 0);
	}
	return rv;
}

void* heap_extract_max(heap *this) {
	heap_item *top = heap_peek(this);
	ASSERT(top, "empty");
	this->size--;
	void* rv = top->data;
	if (this->size > 0) {
		*top = this->arr[this->size];
		heap_heapify_max(this, 0);
	}
	return rv;
}
