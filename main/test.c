#include <test.h>
#include <util.h>
#include <hardware.h>
#include <memory.h>
#include <heap.h>
#include <stack.h>
#include <rawio.h>
#include <priorityq.h>

static void test_pqueue() {
	priorityq pq;
	priorityq_init(&pq, 4, 8, (void*)mem_start_address());

	for (int tasknum = 0; tasknum < 8; tasknum++) {
		for (int priority = 0; priority < 4; priority++) {
			bwprintf(COM2, "PQ PUSH PRIORITY:%d TASK:%d\n", priority, tasknum);
			uint item = priority * 10 + tasknum;
			priorityq_push(&pq, (void*)item, priority);
		}
	}

	for ( int i = 0; i < 32; i++) {
		uint item = (uint)priorityq_pop(&pq);
		bwprintf(COM2, "PQ MAXP PRIORITY:%d TASK:%d\n", item / 10, item%10);
	}
}

static void test_queue() {
	queue* q = (void*)mem_start_address();
	queue_init(q, 5);

	for (int i =0; i < 5; i ++) {
		bwprintf(COM2, "Pushing %d\n", i);
		queue_push(q, (void*)i);
	}

	for (int i = 0; i < 5; i++) {
		bwprintf(COM2, "Popped...");
		uint val = (uint)queue_pop(q);
		bwprintf(COM2, "%d\n", val);
		ASSERT(val == i, "test failed");
	}
}

static void test_pages() {
	int size = 16;
	struct _tag_pages _p;
	pages p = &_p;
	memptr node_table[size];
	pages_init(p, size, (uint) node_table, mem_start_address());
	memptr test[size];
	int i;
	for (i = size - 1; i >= 0; --i) {
		test[i] = pages_get(p);
		ASSERT(test[i], "This should NOT be null. we have more pages.");
		bwprintf(COM2, "page %d: %x\n", i + 1, (uint) test[i]);
	}

	ASSERT(p->head == NULL, "we have used up all the pages. should return 0.");

	bwprintf(COM2, "head after remove %d: %x\n", size, (uint) p->head);
	for (i = size - 1; i >= 0; --i) {
		pages_put(p, test[size - 1 - i]);
		bwprintf(COM2, "head %d: %x\n", i + 1, (uint) p->head);
	}

	ASSERT(p->head == (memptr) p->first_node,
			"we returned the last page last.");
}

static void test_heap() {
	int size = 8;
	struct _tag_heap _h;
	heap h = &_h;
	HEAP_NODE space[size];
	HEAP_INIT(h, (uint) space, size);
	int i;
	for (i = 0; i < size; i++) {
		heap_max_insert(h, i << 2);
		bwprintf(COM2, "inserted: %d, size: %d, top: %d\n", i << 2, h->size,
				HEAP_TOP(h));
	}

	ASSERT(h->arr[0] == (size - 1) << 2,
			"the top is not the largest number inserted");

	for (i = 0; i < size; i++) {
		bwprintf(COM2, "extracted %d, %size %d, top: %d\n", heap_extract_max(h),
				h->size, HEAP_TOP(h));
	}
}

static void test_stack() {
	int size = 8;
	stack s;
	stack_node space[size];
	stack_init(&s, space, size);
	int i;
	for (i = 0; i < size; i++) {
		stack_push(&s, (void*)(i << 2));
		bwprintf(COM2, "inserted: %d, top: %x\n", i << 2, s.top);
	}
	for (i = 0; i < size; i++) {
		bwprintf(COM2, "extracted %d, top: %x\n", stack_pop(&s), s.top);
	}
}

void test_run() {
	bwprintf(COM2, ">>>>>>>>>>>>>>>>>>>>TEST START\n");
	test_pages();
	test_heap();
	test_stack();
	test_queue();
	test_pqueue();
	bwprintf(COM2, ">>>>>>>>>>>>>>>>>>>>TEST END\n");
}
