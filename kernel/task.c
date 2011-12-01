#include <task.h>
#include <memory.h>
#include <stdio.h>
#include <nameserver.h>

static char const * task_state_name[] = {
	"FREE", "NEW", "READY", "RUNNING", "RETIRED",
	"WAITING4SEND", "WAITING4RECEIVE", "WAITING4REPLY", "WAITING4EVENT"
};

static struct _tag_task_descriptor_list {
	task_descriptor head_free;
	task_descriptor *td;
	uint size;
} task_descriptors;

uint get_td_list_size() {
	return task_descriptors.size;
}

void td_print_crash_dump() {
//	bwprintf(1, "\x1B[2J"); // clear console
	int tdsize = get_td_list_size();

	for (int i = 0; i < tdsize; i++) {
		task_descriptor *td = task_descriptors.td + i;
		if (td->state != TD_STATE_FREE && td->state != TD_STATE_RETIRED) {
			char waitinfostr[100] = { '\0' };
			if (td->state == TD_STATE_WAITING4RECEIVE) {
				// sprintf(waitinfostr, ", sending to: %d", td_find_send_target(td));
			}
			if (td->state == TD_STATE_WAITING4REPLY) {
				sprintf(waitinfostr, ", last_receiver: %d", td->last_receiver);
			}
			char *taskname = nameserver_get_name(td->id);
			if (!taskname) taskname = "{noname}";

			bwprintf(1, "Task %d %s (p:%d, %s%s): ",
					td->id, taskname, td->priority, task_state_name[td->state], waitinfostr);
			print_stack_trace(td->registers.r[REG_FP], 0);
			bwprintf(1, "\n");
		}
	}
}

void td_init(uint task_list_size) {
	task_descriptors.td = kmalloc(task_list_size * sizeof(task_descriptor));
	task_descriptors.size = task_list_size;
	// initialize free queue
	task_descriptor *head_free = &task_descriptors.head_free;
	td_clear_siblings(head_free);
	td_clear_children(head_free);
	// put the free descriptors in the free queue
	task_descriptor *td = task_descriptors.td;
	for (int i = 0; i < task_list_size; ++i, ++td) {
		td->id = i;
		td->state = TD_STATE_FREE;
		td->registers.r[REG_FP] = 0;
		td_clear_siblings(td);
		td_clear_children(td);
		td_push_child(head_free, td);
		td->last_receiver = -1;
	}

	PRINT("task_list_size: %d", get_td_list_size());
}

int td_index(int tid) {
	return tid & 0xffff;
}

int td_impossible(int tid) {
	int i = td_index(tid);
	return i < 0 || i >= task_descriptors.size;
}

void td_clear_siblings(task_descriptor *td) {
	td->_next = NULL;
}

void td_clear_children(task_descriptor *td) {
	td->_head_child = NULL;
	td->_tail_child = NULL;
}

int td_has_children(task_descriptor *td) {
	return td->_head_child != NULL;
}

void td_push_child(task_descriptor *td, task_descriptor *child) {
	ASSERT(!child->_next, "this task descriptor is already on a list");
	if (td_has_children(td)) {
		task_descriptor *tail = td->_tail_child;
		tail->_next = child;
		td->_tail_child = child;
	} else {
		td->_head_child = child;
		td->_tail_child = child;
	}
}

task_descriptor *td_shift_child(task_descriptor *td) {
	ASSERT(td_has_children(td), "no children");
	task_descriptor *child = td->_head_child;
	if (child->_next) {
		td->_head_child = child->_next;
	} else {
		td_clear_children(td);
	}
	td_clear_siblings(child);
	return child;
}

task_descriptor *td_new() {
	task_descriptor *td = td_shift_child(&task_descriptors.head_free);
	td->state = TD_STATE_NEW;
	return td;
}

void td_free(task_descriptor *td) {
	td->id += 0x10000; // increment generation
	td->registers.r[REG_FP] = 0;
	if (td->id >= 0) {
		td->state = TD_STATE_FREE;
		td_clear_siblings(td);
		td_clear_children(td);
		td_push_child(&task_descriptors.head_free, td);
	} else {
		td->state = TD_STATE_RETIRED;
	}
}

task_descriptor *td_find(uint id) {
	int i = td_index(id);
	if (i >= task_descriptors.size) return NULL;
	task_descriptor *td = task_descriptors.td + i;
	return td->id == id ? td : NULL;
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
