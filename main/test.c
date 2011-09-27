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
	priorityq_init(&pq, 4, 8, (void*) mem_start_address());

	bwprintf(COM2, "PQ pushing ");

	for (int tasknum = 0; tasknum < 8; tasknum++) {
		for (int priority = 0; priority < 4; priority++) {
			bwprintf(COM2, " [%d,%d]", priority, tasknum);
			uint item = priority * 10 + tasknum;
			priorityq_push(&pq, (void*) item, priority);
		}
	}

	bwprintf(COM2, "\nPQ popping ");

	for (int i = 0; i < 32; i++) {
		uint item = (uint) priorityq_pop(&pq);
		bwprintf(COM2, " [%d,%d]", item / 10, item % 10);
	}

	bwprintf(COM2, "\n");
}

static void test_queue() {
	queue* q = (void*) mem_start_address();
	queue_init(q, 5);

	bwprintf(COM2, "Queue Pushing ");

	for (int i = 0; i < 5; i++) {
		bwprintf(COM2, " %d", i);
		queue_push(q, (void*) i);
	}

	ASSERT(QUEUE_FULL(q), "queue not full!?");

	bwprintf(COM2, "\nPopping...");

	for (int i = 0; i < 5; i++) {
		uint val = (uint) queue_pop(q);
		bwprintf(COM2, " %d", val);
		ASSERT(val == i, "test failed");
	}

	bwprintf(COM2, " \n");

	ASSERT(QUEUE_EMPTY(q), "queue not empty!?");

}

static void test_pages() {
	int size = 16;
	struct _tag_pages _p;
	pages p = &_p;
	memptr node_table[size];
	pages_init(p, size, (uint) node_table, (void *) mem_start_address());
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

	bwprintf(COM2, "Heap inserting...");
	for (i = 0; i < size; i++) {
		int item = i << 2;
		heap_max_insert(h, i << 2);
		bwprintf(COM2, " %d", i << 2);
		ASSERT(HEAP_TOP(h) == item, "wrong top item");
	}

	ASSERT(h->arr[0] == (size - 1) << 2,
			"the top is not the largest number inserted");

	bwprintf(COM2, "\nHeap popping...");
	for (i = 0; i < size; i++) {
		int item = heap_extract_max(h);
		bwprintf(COM2, " %d", item);
	}

	bwputc(COM2, '\n');
}

static void test_stack() {
	int size = 8;
	stack s;
	stack_node space[size];
	stack_init(&s, space, size);
	int i;

	bwprintf(COM2, "Stack pushing...");
	for (i = 0; i < size; i++) {
		stack_push(&s, (void*) (i << 2));
		bwprintf(COM2, " %d", i << 2);
	}

	bwprintf(COM2, "\nStack popping...");
	for (i = 0; i < size; i++) {
		bwprintf(COM2, " %d", stack_pop(&s));
	}
	bwputc(COM2, '\n');
}

void test_run() {
	bwprintf(COM2, ">>>>>>>>>>>>>>>>>>>>TEST START\n");
	//test_pages();
	test_heap();
	test_stack();
	test_queue();
	test_pqueue();
	bwprintf(COM2, "<<<<<<<<<<<<<<<<<<<<TEST END\n");
}
