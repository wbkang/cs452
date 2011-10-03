#include <task.h>
#include <memory.h>

static struct _tag_task_descriptor_list {
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
	if (td == td->_next) { // ran out
		return NULL;
	}
	TD_REMOVE(td);
	return td;
}

void td_free(task_descriptor *td) {
	td->id += 0x10000; // increment generation
	TD_REMOVE(td);
	if (td->id >= 0) {
		TD_APPEND(&task_descriptors.head_free, td);
	}
}

task_descriptor *td_find(uint id) {
	task_descriptor *td = &task_descriptors.td[id & 0xFFFF];
	return td->id == id ? td : NULL;
}

void reginfo(register_set *reg) {
	TRACE("reginfo:");
	TRACE("\treg: %x", reg);
	TRACE("\tspsr: %x", reg->spsr);
	for (int i = 0; i < 13; i++) {
		TRACE("\tr%d: %x", i, reg->r[i]);
	}
	TRACE("\tsp: %x", reg->r[13]);
	TRACE("\tlr: %x", reg->r[14]);
	TRACE("\tpc: %x", reg->r[15]);
}
