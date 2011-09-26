#include <task.h>

static int last_used_tid;

static struct _tag_task_descriptor_list {
		task_descriptor head_taken;
		task_descriptor head_free;
		task_descriptor td[TASK_LIST_SIZE];
} task_descriptors;

#define TD_REMOVE(td) { \
	(td)->_prev->_next = (td)->_next; \
	(td)->_next->_prev = (td)->_prev; \
}

#define TD_APPEND(ref, td) { \
	(td)->_prev = (ref); \
	(td)->_next = (td)->_prev->_next; \
	(td)->_prev->_next = (td); \
	(td)->_next->_prev = (td); \
}

#define TD_MOVE(ref, td) { \
	TD_REMOVE(td); \
	TD_APPEND(ref, td); \
}


void td_init() {
	last_used_tid = 0;

	// initialize taken queue
	task_descriptor *head_taken = &task_descriptors.head_taken;

	head_taken->_prev = head_taken;
	head_taken->_next = head_taken;

	// initialize free queue
	task_descriptor *head_free = &task_descriptors.head_free;

	head_free->_prev = head_free;
	head_free->_next = head_free;

	// put the free descriptors in the free queue
	task_descriptor *td = task_descriptors.td;

	for (int i = TASK_LIST_SIZE - 1; i != -1; --i, ++td) {
		td->id = i;
		TD_APPEND(head_free, td);
	}
}

task_descriptor *td_new() {
	task_descriptor *td = task_descriptors.head_free._next;
	ASSERT(td != &task_descriptors.head_free, "no more empty task descriptors");

	TD_MOVE(&task_descriptors.head_taken, td);

	td->id = ++last_used_tid;
	return td;
}

void td_delete(task_descriptor *td) {
	TD_MOVE(&task_descriptors.head_free, td);
}
