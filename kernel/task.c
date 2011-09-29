#include <task.h>

static struct _tag_task_descriptor_list {
	//task_descriptor head_taken;
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

void td_init() {
	// initialize taken queue
	//task_descriptor *head_taken = &task_descriptors.head_taken;

	//head_taken->_prev = head_taken;
	//head_taken->_next = head_taken;

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
	// TD_MOVE(&task_descriptors.head_taken, td);
	TD_REMOVE(td);
	return td;
}

void td_free(task_descriptor *td) {
	td->id += 1 << 16;
	TD_REMOVE(td);
	TD_APPEND(&task_descriptors.head_free, td);
}

task_descriptor *td_find(uint id) {
	task_descriptor *td = &task_descriptors.td[id & 0x0000FFFF];
	return td->id == id ? td : NULL;
}

void reginfo(register_set *reg) {
	TRACE("reginfo:\n");
	TRACE("\tspsr: %x\n", reg->spsr);
	for (int i = 0; i < 13; i++) {
		TRACE("\tr%d: %x\n", i, reg->r[i]);
	}
	TRACE("\tsp: %x\n", reg->r[13]);
	TRACE("\tlr: %x\n", reg->r[14]);
	TRACE("\tpc: %x\n", reg->r[15]);
}
