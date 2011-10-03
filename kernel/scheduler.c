#include <scheduler.h>
#include <priorityq.h>
#include <memory.h>

static task_descriptor *running;
static priorityq *ready_queue;
static int reschedule;

void scheduler_init() {
	running = NULL;
	reschedule = TRUE;
	TRACE("\tready queue of size %d initialized", TASK_LIST_SIZE);
	ready_queue = priorityq_new(TASK_LIST_SIZE, NUM_PRIORITY);
}

task_descriptor *scheduler_running() {
	return running;
}

void scheduler_ready(task_descriptor *td) {
	td->state = TD_STATE_READY;
	priorityq_push(ready_queue, td, td->priority);
}

int scheduler_empty() {
	return reschedule && PRIORITYQ_EMPTY(ready_queue);
}

task_descriptor *scheduler_get() {
	if (reschedule) {
		running = priorityq_pop(ready_queue);
		running->state = TD_STATE_RUNNING;
	} else {
		reschedule = TRUE;
	}
	return (task_descriptor *) running;
}

void scheduler_killme() {
	free_user_memory(running);
	td_free(running);
	running = NULL;
}

void scheduler_move2ready() {
	ASSERT(running, "no task running to move to ready");
	scheduler_ready(running);
}

void scheduler_runmenext() {
	reschedule = FALSE;
}
