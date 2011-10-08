#include <scheduler.h>
#include <priorityq.h>
#include <memory.h>

static task_descriptor *running;
static priorityq *ready_queue;
static int reschedule;

void scheduler_init() {
	running = NULL;
	reschedule = TRUE;
	ready_queue = priorityq_new(TASK_LIST_SIZE, NUM_PRIORITY);
}

inline task_descriptor *scheduler_running() {
	return running;
}

inline int scheduler_empty() {
	return reschedule && priorityq_empty(ready_queue);
}

inline task_descriptor *scheduler_get() {
	if (reschedule) {
		running = priorityq_pop(ready_queue);
		running->state = TD_STATE_RUNNING;
	} else {
		reschedule = TRUE;
	}
	return running;
}

inline void scheduler_move2ready() {
	scheduler_ready(running);
}

inline void scheduler_runmenext() {
	reschedule = FALSE;
}

inline void scheduler_ready(task_descriptor *td) {
	td->state = TD_STATE_READY;
	priorityq_push(ready_queue, td, td->priority);
}

inline void scheduler_wait4send(task_descriptor *receiver) {
	receiver->state = TD_STATE_WAITING4SEND;
}

inline void scheduler_wait4receive(task_descriptor *receiver, task_descriptor *sender) {
	sender->state = TD_STATE_WAITING4RECEIVE;
	td_list_push(receiver, sender);
}

inline void scheduler_wait4reply(task_descriptor *sender) {
	sender->state = TD_STATE_WAITING4REPLY;
}
