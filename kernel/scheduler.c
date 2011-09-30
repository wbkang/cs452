#include <scheduler.h>
#include <priorityq.h>
#include <memory.h>

static priorityq *ready_queue;

void scheduler_init() {
	TRACE("\tready queue of size %d initialized\n", TASK_LIST_SIZE);
	ready_queue = priorityq_new(TASK_LIST_SIZE, NUM_PRIORITY);
}

void scheduler_ready(task_descriptor *td) {
	priorityq_push(ready_queue, td, td->priority);
}

int scheduler_empty() {
	return PRIORITYQ_EMPTY(ready_queue);
}

task_descriptor *scheduler_get() {
	return priorityq_pop(ready_queue);
}
