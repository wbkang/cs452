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

static inline void td_list_close(task_descriptor *td);

uint get_td_list_size() {
	return task_descriptors.size;
}

static int td_find_send_target(task_descriptor *td) {
	ASSERTNOTNULL(td);
	ASSERT(td->state == TD_STATE_WAITING4RECEIVE, "task %d in a non-rcv-blocked state: %d", td->id, td->state);

	task_descriptor *sender = td;
	task_descriptor *cur_td = sender->_next;

	while (cur_td != sender) {
		if (cur_td->state != TD_STATE_WAITING4RECEIVE) {
			return cur_td->id;
		}
	}

	return -1; // not found!! this should be unreachable
}

void td_print_crash_dump() {
	bwprintf(1, "\x1B[2J"); // clear console
	int tdsize = get_td_list_size();

	for (int i = 0; i < tdsize; i++) {
		task_descriptor *td = task_descriptors.td + i;
		if (td->state != TD_STATE_FREE && td->state != TD_STATE_RETIRED) {
			char waitinfostr[100];
			if (td->state == TD_STATE_WAITING4RECEIVE) {
				sprintf(waitinfostr, ", sending to: %d", td_find_send_target(td));
			} else {
				waitinfostr[0] = '\0';
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
	td_list_close(head_free);
	// put the free descriptors in the free queue
	task_descriptor *td = task_descriptors.td;
	for (int i = 0; i < task_list_size; ++i, ++td) {
		td->id = i;
		td->state = TD_STATE_FREE;
		td->registers.r[REG_FP] = 0;
		td_list_push(head_free, td);
	}

	PRINT("task_list_size: %d", get_td_list_size());
}

inline int td_index(int tid) {
	return tid & 0xffff;
}

inline int td_impossible(int tid) {
	int i = td_index(tid);
	return i < 0 || i >= task_descriptors.size;
}

inline int td_list_empty(task_descriptor *td) {
	return td->_prev == td;
}

static inline void td_list_remove(task_descriptor *td) {
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

inline task_descriptor *td_list_pop(task_descriptor *head) {
	task_descriptor *td = head->_prev;
	ASSERT(td != head, "td list empty");
	td_list_remove(td);
	return td;
}

static inline void td_list_close(task_descriptor *td) {
	td->_prev = td;
	td->_next = td;
}

inline task_descriptor *td_new() {
	task_descriptor *td = td_list_pop(&task_descriptors.head_free);
	td->state = TD_STATE_NEW;
	return td;
}

inline void td_free(task_descriptor *td) {
	td->id += 0x10000; // increment generation
	td->registers.r[REG_FP] = 0;
	if (td->id >= 0) {
		td_list_push(&task_descriptors.head_free, td);
		td->state = TD_STATE_FREE;
	} else {
		td->state = TD_STATE_RETIRED;
	}
}

inline task_descriptor *td_find(uint id) {
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
