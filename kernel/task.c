#include <task.h>

static struct _tag_task_descriptor_list {
	task_descriptor head_free;
	task_descriptor td[TASK_LIST_SIZE];
} task_descriptors;

void td_init() {
	// initialize free queue
	task_descriptor *head_free = &task_descriptors.head_free;
	TD_CLOSE(head_free);
	// put the free descriptors in the free queue
	task_descriptor *td = task_descriptors.td;
	for (int i = 0; i < TASK_LIST_SIZE; ++i, ++td) {
		td->id = i;
		td->state = TD_STATE_FREE;
		TD_PUSH(head_free, td);
	}
}

task_descriptor *td_new() {
	task_descriptor *head = &task_descriptors.head_free;
	if (TD_EMPTYLIST(head)) return NULL;
	task_descriptor *rv = td_pop(head);
	TD_CLOSE(rv);
	rv->state = TD_STATE_NEW;
	return rv;
}

void td_free(task_descriptor *td) {
	td->id += 0x10000; // increment generation
	TD_REMOVE(td);
	if (td->id >= 0) {
		TD_PUSH(&task_descriptors.head_free, td);
		td->state = TD_STATE_FREE;
	} else {
		td->state = TD_STATE_RETIRED;
	}
}

task_descriptor *td_find(uint id) {
	task_descriptor *td = &task_descriptors.td[TD_INDEX(id)];
	return td->id == id ? td : NULL;
}

task_descriptor *td_pop(task_descriptor *head) {
	task_descriptor *td = head->_prev;
	if (td == head) return NULL;
	TD_REMOVE(td);
	return td;
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
