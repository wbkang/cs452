#include <test.h>
#include <util.h>
#include <hardware.h>
#include <memory.h>
#include <heap.h>
#include <stack.h>
#include <rawio.h>
#include <priorityq.h>

#define TEST_START(name) bwprintf(COM2, "%s... ", name)

#define TEST_END(name) bwprintf(COM2, "looks good\n")

#define EXPECT(expected, got) { \
	if (got != expected) { \
		bwprintf(COM2, "ERROR!!!\nExpected %d but got %d (0x%x)", expected, got, got); \
		die(); \
	} \
}

static void test_stack() {
	TEST_START("stack");
	int size = 1337;
	memptr p = mem_top();
	stack *s = new_stack(size, &p);
	int i;
	void *got, *expected;
	for (i = 0; i < size; i++) {
		stack_push(s, (void *) (1 << i));
	}
	for (i = size - 1; i != -1; i--) {
		got = stack_pop(s);
		expected = (void *) (1 << i);
		EXPECT(expected, got);
	}
	TEST_END("stack");
}

static void test_queue() {
	TEST_START("queue");
	int size = 1337;
	memptr p = mem_top();
	queue *q = queue_init(size, &p);
	void *got, *expected;
	for (int i = 0; i < size; i++) {
		queue_push(q, (void*) (1 << i));
	}
	ASSERT(QUEUE_FULL(q), "ERROR!!!\nInserted max no of items yet not full!");
	for (int i = 0; i < size; i++) {
		got = queue_pop(q);
		expected = (void *) (1 << i);
		EXPECT(expected, got);
	}
	TEST_END("queue");
}

static void test_pqueue() {
	TEST_START("priority queue");
	int size = 1337;
	int num_priorities = 1337;
	memptr p = mem_top();
	priorityq *pq = priorityq_init(size, num_priorities, &p);

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

	TEST_END("priority queue");
}

//static void test_pages() {
//	int size = 16;
//	struct _tag_pages _p;
//	pages p = &_p;
//	memptr node_table[size];
//	pages_init(p, size, (uint) node_table, mem_top());
//	memptr test[size];
//	int i;
//	for (i = size - 1; i >= 0; --i) {
//		test[i] = pages_get(p);
//		ASSERT(test[i], "This should NOT be null. we have more pages.");
//		bwprintf(COM2, "page %d: %x\n", i + 1, (uint) test[i]);
//	}
//
//	ASSERT(p->head == NULL, "we have used up all the pages. should return 0.");
//
//	bwprintf(COM2, "head after remove %d: %x\n", size, (uint) p->head);
//	for (i = size - 1; i >= 0; --i) {
//		pages_put(p, test[size - 1 - i]);
//		bwprintf(COM2, "head %d: %x\n", i + 1, (uint) p->head);
//	}
//
//	ASSERT(p->head == (memptr) p->first_node,
//			"we returned the last page last.");
//}

static void test_heap() {
	TEST_START("heap");
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

	TEST_END("heap");
}

void test_run() {
	mem_init();
	bwprintf(COM2, "################ DIAGNOSTICS ################\n");
	//test_pages();
	test_stack();
	test_queue();
	test_heap();
	test_pqueue();
	bwprintf(COM2, "################ END OF DIAGNOSTICS ################\n");
}
