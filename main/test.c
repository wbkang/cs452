#include <test.h>
#include <util.h>
#include <hardware.h>
#include <heap.h>
#include <stack.h>
#include <priorityq.h>
#include <string.h>
#include <buffer.h>

void mem_reset(); // from memory.c

static void test_stack() {
	TEST_START();
	int size = 111;
	stack *s = stack_new(size);
	void *got, *expected;
	for (int i = 0; i < size; i++) {
		stack_push(s, (void*) (1 << i));
	}
	for (int i = size - 1; i != -1; i--) {
		got = stack_pop(s);
		expected = (void*) (1 << i);
		EXPECT(expected, got);
	}
	TEST_END();
}

static void test_buffer() {
	TEST_START();
	int size = 1024;
	struct {
		int a, b;
	} temp[size], rv;
	buffer *q = buffer_new(size, sizeof rv);

	for (int i = 0; i < size; i++) {
		temp[i].a = random();
		temp[i].b = random();
		buffer_put(q, temp + i);
	}
	ASSERT(buffer_full(q), "ERROR!!!\nInserted max no of items yet not full!");
	for (int i = 0; i < size; i++) {
		buffer_get(q, &rv);
		EXPECT(temp[i].a, rv.a);
		EXPECT(temp[i].b, rv.b);
	}
	TEST_END();
}

static void test_queue() {
	TEST_START();
	int size = 111;
	queue *q = queue_new(size);
	void *got, *expected;
	for (int i = 0; i < size; i++) {
		queue_put(q, (void*) (1 << i));
	}
	ASSERT(queue_full(q), "ERROR!!!\nInserted max no of items yet not full!");
	for (int i = 0; i < size; i++) {
		got = queue_get(q);
		expected = (void*) (1 << i);
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
			priorityq_put(pq, (void*) (i * (num_priorities + 1) + j), j);
		}
	}
	for (int j = num_priorities - 1; j != -1; j--) {
		for (int i = 0; i < size; i++) {
			expected = (void*) (i * (num_priorities + 1) + j);
			got = priorityq_get(pq);
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
	for (int i = size - 1; i != -1; i--) {
		heap_insert_min(h, (void*) i, 11 * i);
	}
	for (int i = 0; i < size; i++) {
		got = heap_extract_min(h);
		expected = (void*) i;
		EXPECT(expected, got);
	}
	TEST_END();
}

static void test_memcpy() {
	TEST_START();
	int size = 1024;
	int numtests = 100;
	struct _tag_memcpy_test_data {
		int data[size];
	} src, dst;
	for (int i = 0; i < numtests; i++) {
		int test_size = random() % size;
		for (int j = 0; j < test_size; j++) {
			src.data[j] = random();
		}
		memcpy(dst.data, src.data, 4 * test_size);
		for (int j = 0; j < test_size; j++) {
			EXPECT(src.data[j], dst.data[j]);
		}
	}
	TEST_END();
}

void test_run() {
	mem_reset();
	test_stack();
	test_queue();
	test_buffer();
	test_priorityq();
	test_heap();
	test_memcpy();
	mem_reset();
}
