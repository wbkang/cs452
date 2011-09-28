#include <test.h>
#include <util.h>
#include <hardware.h>
#include <memory.h>
#include <heap.h>
#include <stack.h>
#include <rawio.h>
#include <priorityq.h>

#define TEST_START(name) bwprintf(COM2, "%s ... ", name)

#define TEST_END(name) bwprintf(COM2, "looks good\n")

#define EXPECT(expected, got) { \
	if (got != expected) { \
		bwprintf(COM2, "ERROR!!!\nExpected %d but got %d (0x%x)", expected, got, got); \
		die(); \
	} \
}

static void test_stack() {
	TEST_START("stack");
	int size = 111;
	memptr p = mem_top();
	stack *s = stack_new(size, &p);
	void *got, *expected;
	for (int i = 0; i < size; i++) {
		stack_push(s, (void *) (1 << i));
	}
	for (int i = size - 1; i != -1; i--) {
		got = stack_pop(s);
		expected = (void *) (1 << i);
		EXPECT(expected, got);
	}
	TEST_END("stack");
}

static void test_queue() {
	TEST_START("queue");
	int size = 111;
	memptr p = mem_top();
	queue *q = queue_new(size, &p);
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

static void test_priorityq() {
	TEST_START("priority queue");
	int size = 111;
	int num_priorities = 32;
	memptr p = mem_top();
	priorityq *pq = priorityq_new(size, num_priorities, &p);
	void *got, *expected;
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < num_priorities; j++) {
			priorityq_push(pq, (void*) (i * (num_priorities + 1) + j), j);
		}
	}
	for (int j = num_priorities - 1; j != -1; j--) {
		for (int i = 0; i < size; i++) {
			expected = (void*) (i * (num_priorities + 1) + j);
			got = priorityq_pop(pq);
			EXPECT(expected, got);
		}
	}
	TEST_END("priority queue");
}

static void test_heap() {
	TEST_START("heap");
	int size = 111;
	memptr p = mem_top();
	heap *h = heap_new(size, &p);
	void *got, *expected;
	for (int i = 0; i < size; i++) {
//		bwprintf(COM2, " %d", i);
		heap_max_insert(h, (void *) i, 11 * i);
	}
//	bwprintf(COM2, "popping");
	for (int i = size - 1; i != -1; i--) {
		got = heap_extract_max(h);
		expected = (void *) i;
//		bwprintf(COM2, " %d", (int) i);
		EXPECT(expected, got);
	}
	TEST_END("heap");
}

void test_run() {
	mem_init();
	bwprintf(COM2, "################ DIAGNOSTICS ################\n");
	test_stack();
	test_queue();
	test_priorityq();
	mem_init();
	test_heap();
	bwprintf(COM2, "################ END OF DIAGNOSTICS ################\n");
}
