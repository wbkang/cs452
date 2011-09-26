#pragma once

#include <util.h>
#include <hardware.h>


#define TASK_LIST_SIZE 16


typedef struct _tag_register_set {
	int spsr;
	int registers[15]; // r0-r12, sp and lr
} register_set;

typedef struct _tag_task_descriptor {
	int id;
	uint state;
	uint priority;
	int parent_id; // should this be a pointer to the parent td?
	register_set registers; // r0-r12, sp and lr; these should be on user stack?
	struct _tag_task_descriptor *_prev;
	struct _tag_task_descriptor *_next;
} task_descriptor;




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

/*
 * I just realized that I could have used a stack as well. To remove from the
 * middle you simply memcpy the top of the stack to the middle and pop. This
 * will be simpler and use less space, but there will be a cost of memcpy on
 * every delete.
 */


void td_init();

task_descriptor* td_new();

void td_delete(task_descriptor *td);
