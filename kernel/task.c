#include <task.h>

void td_init(task_descriptor_list *tdlist) {
	// initialize taken queue
	task_descriptor *head_taken = &tdlist->head_taken;
	head_taken->prev = head_taken;
	head_taken->next = head_taken;
	// initialize free queue
	task_descriptor *head_free = &tdlist->head_free;
	head_free->prev = head_free;
	head_free->next = head_free;
	// put the free descriptors in the free queue
	task_descriptor *td = tdlist->td;
	for (int i = TASK_LIST_SIZE - 1; i != -1; --i, ++td) {
		td->id = i;
		TD_APPEND(head_free, td);
	}
}

task_descriptor *td_new(task_descriptor_list *tdlist) {
	task_descriptor *head_free = &tdlist->head_free;
	task_descriptor *td = head_free->next;
	ASSERT(td != head_free, "no more empty task descriptors");
	TD_MOVE(&tdlist->head_taken, td);
	return td;
}

void td_delete(task_descriptor_list *tdlist, task_descriptor *td) {
	// ASSERT(td is in the taken queue, "...") // no efficient way to find out
	TD_MOVE(&tdlist->head_free, td);
}
