#pragma once

#include <util.h>
#include <hardware.h>

typedef struct _tag_register_set {
		int registers[15]; // r0-r12, sp and lr
}* register_set;

struct _tag_task_descriptor {
		uint id;
		uint state;
		uint priority;
		uint parent_id;
		memptr stack;
		uint spsr;
		uint rv;
		struct _tag_task_descriptor *prev;
		struct _tag_task_descriptor *next;
		int registers[15]; // r0-r12, sp and lr
};

typedef struct _tag_task_descriptor task_descriptor;

/*
 * The task descriptor list must support the following operations in O(1)
 * 		return a pointer to a free task descriptor
 * 		given a pointer to a task descriptor, free that descriptor for later use
 * 	Furthermore, the list must only use static memory.
 *
 * 	I chose to implement the list as a doubly linked list.
 *
 * 	Using static memory means that all possible descriptors must be initialized
 * 	at the beginning and then given out and taken back during execution.
 *
 * 	To do this I use two doubly linked lists. One stores the taken descriptors,
 * 	and the other stores the free ones. The required operations simply move the
 * 	descriptors from one list to another.
 *
 * 	During initialization the taken list is initialized to empty, while the free
 * 	list is populated with the maximum number of dummy descriptors.
 *
 * 	The first two task descriptors are sentinel values. The first one is the
 * 	head of the taken list, and the second one is the head of the free list.
 */

#define TD_FULL(tdl) (TD_HEAD_FREE(tdl)->next == TD_HEAD_FREE(tdl))
#define TD_HEAD_TAKEN(tdl) tdl[0]
#define TD_HEAD_FREE(tdl) tdl[1]

#define TD_REMOVE(td) { \
	td->prev->next = td->next; \
	td->next->prev = td->prev; \
}

#define TD_APPEND(ref, td) {\
	td->prev = ref; \
	td->next = td->prev->next; \
	td->prev->next = td; \
	td->next->prev = td; \
}
