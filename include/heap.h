#include <util.h>

#define HEAP_NODE uint
#define HEAP_NODEMAG(n) (n)
#define HEAP_GT(h, i, j) (HEAP_NODEMAG(h->arr[i]) > HEAP_NODEMAG(h->arr[j]))
#define HEAP_GE(h, i, j) (HEAP_NODEMAG(h->arr[i]) >= HEAP_NODEMAG(h->arr[j]))

#define HEAP_PARENT(i) ((i) >> 1) // floor(i / 2)
#define HEAP_LEFTCHILD(i) ((i) << 1) // 2 * i
#define HEAP_RIGHTCHILD(i) (((i) << 1) | 1) // 2 * i + 1
#define HEAP_TOP(h) h->arr[0]

#define HEAP_EXCHANGE(h, i, j) { \
	HEAP_NODE tmp = h->arr[i]; \
	h->arr[i] = h->arr[j]; \
	h->arr[j] = tmp; \
}

typedef struct _tag_heap {
	HEAP_NODE *arr;
	uint size;
	uint max_size;
}*heap;

#define HEAP_INIT(h, addr, size) {\
	h->arr = (HEAP_NODE *) addr; \
	h->size = 0; \
	h->max_size = size; \
}

void heap_max_heapify(heap h, uint i);

void heap_max_heapify2(heap h, uint i);

HEAP_NODE heap_extract_max(heap h);

void heap_increase_key(heap h, uint i, HEAP_NODE key);

void heap_max_insert(heap h, HEAP_NODE key);
