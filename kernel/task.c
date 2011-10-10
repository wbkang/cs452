#include <task.h>

static struct _tag_task_descriptor_list {
	task_descriptor head_free;
	task_descriptor td[TASK_LIST_SIZE];
} task_descriptors;

void td_init() {
	// initialize free queue
	task_descriptor *head_free = &task_descriptors.head_free;
	td_list_close(head_free);
	// put the free descriptors in the free queue
	task_descriptor *td = task_descriptors.td;
	for (int i = 0; i < TASK_LIST_SIZE; ++i, ++td) {
		td->id = i;
		td->state = TD_STATE_FREE;
		td_list_push(head_free, td);
	}
}

inline int td_index(int tid) {
	return tid & 0xffff;
}

inline int td_impossible(int tid) {
	return tid < 0 || (td_index(tid) >= TASK_LIST_SIZE);
}

inline int td_list_empty(task_descriptor *td) {
	return td->_prev == td;
}

inline void td_list_close(task_descriptor *td) {
	td->_prev = td;
	td->_next = td;
}

inline void td_list_remove(task_descriptor *td) {
	td->_prev->_next = td->_next;
	td->_next->_prev = td->_prev;
	td_list_close(td);
}

inline void td_list_push(task_descriptor *head, task_descriptor *td) {
	td->_prev = head;
	td->_next = head->_next;
	td->_next->_prev = td;
	head->_next = td;
}

task_descriptor *td_new() {
	task_descriptor *head = &task_descriptors.head_free;
	if (td_list_empty(head)) return NULL;
	task_descriptor *rv = td_pop(head);
	td_list_close(rv);
	rv->state = TD_STATE_NEW;
	return rv;
}

void td_free(task_descriptor *td) {
	td->id += 0x10000; // increment generation
	td_list_remove(td);
	if (td->id >= 0) {
		td_list_push(&task_descriptors.head_free, td);
		td->state = TD_STATE_FREE;
	} else {
		td->state = TD_STATE_RETIRED;
	}
}

task_descriptor *td_find(uint id) {
	int i = td_index(id);
	if (i >= TASK_LIST_SIZE) return NULL;
	task_descriptor *td = task_descriptors.td + i;
	return td->id == id ? td : NULL;
}

task_descriptor *td_pop(task_descriptor *head) {
	task_descriptor *td = head->_prev;
	if (td == head) return NULL;
	td_list_remove(td);
	return td;
}

void reginfo(register_set *reg) {
	TRACE("reginfo:");
	TRACE("\treg: %x", reg);
	TRACE("\tspsr: %x", reg->spsr);
	for (int i = 0; i < 13; i++) TRACE("\tr%d: %x", i, reg->r[i]);
	TRACE("\tsp: %x", reg->r[13]);
	TRACE("\tlr: %x", reg->r[14]);
	TRACE("\tpc: %x", reg->r[15]);
}
