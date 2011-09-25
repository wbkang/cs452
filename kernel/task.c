#include <task.h>

void td_init(task_descriptor **tdlist, uint size) {
	// initialize taken queue
	task_descriptor *head_taken = TD_HEAD_TAKEN(tdlist);
	head_taken->prev = head_taken;
	head_taken->next = head_taken;
	// initialize free queue
	task_descriptor *head_free = TD_HEAD_FREE(tdlist);
	head_free->prev = head_free;
	head_free->next = head_free;
	// put the free descriptors in the free queue
	/*
	 * This initialization can be avoided by keeping track of the highest index
	 * of a given-out task descriptor. If the free queue is empty and the
	 * highest index is less than the max index we simply increment the index,
	 * initialize the task descriptor at that index and return it.
	 *
	 * Doing the initialization here makes the td_new code faster.
	 */
	for (uint i = size - 1; i > 1; --i) {
		TD_APPEND(head_free, tdlist[i]);
	}
}

task_descriptor *td_new(task_descriptor **tdlist) {
	ASSERT(!TD_FULL(tdlist), "no more empty task descriptors");
	task_descriptor *td = TD_HEAD_FREE(tdlist)->next;
	TD_REMOVE(td);
	TD_APPEND(TD_HEAD_TAKEN(tdlist), td);
	return td;
}

void td_delete(task_descriptor **tdlist, task_descriptor *td) {
	// ASSERT(td is in the taken queue, "...") // no efficient way to find out
	TD_REMOVE(td);
	TD_APPEND(TD_HEAD_FREE(tdlist), td);
}
