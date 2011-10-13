#include <test.h>
#include <util.h>
#include <hardware.h>
#include <memory.h>
#include <heap.h>
#include <stack.h>
#include <rawio.h>
#include <priorityq.h>

static void test_stack() {
	TEST_START();
	int size = 111;
	stack *s = stack_new(size);
	void *got, *expected;
	for (int i = 0; i < size; i++) {
		stack_push(s, (void *) (1 << i));
	}
	for (int i = size - 1; i != -1; i--) {
		got = stack_pop(s);
		expected = (void *) (1 << i);
		EXPECT(expected, got);
	}
	TEST_END();
}

static void test_queue() {
	TEST_START();
	int size = 111;
	queue *q = queue_new(size);
	void *got, *expected;
	for (int i = 0; i < size; i++) {
		queue_push(q, (void*) (1 << i));
	}
	ASSERT(queue_full(q), "ERROR!!!\nInserted max no of items yet not full!");
	for (int i = 0; i < size; i++) {
		got = queue_pop(q);
		expected = (void *) (1 << i);
		EXPECT(expected, got);
	}
	TEST_END();
}

static void test_priorityq() {
	TEST_START();
	int size = 111;
	int num_priorities = 32;
	priorityq *pq = priorityq_new(size, num_priorities);
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
	TEST_END();
}

static void test_heap() {
	TEST_START();
	int size = 111;
	heap *h = heap_new(size);
	void *got, *expected;
	for (int i = 0; i < size; i++) {
		heap_insert_max(h, (void *) i, 11 * i);
	}
	for (int i = size - 1; i != -1; i--) {
		got = heap_extract_max(h);
		expected = (void *) i;
		EXPECT(expected, got);
	}
	TEST_END();
}

void test_run() {
	mem_reset();
	test_stack();
	test_queue();
	test_priorityq();
	test_heap();
	mem_reset();
}
