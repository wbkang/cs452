#include <scheduler.h>
#include <priorityq.h>
#include <memory.h>

volatile static task_descriptor *running;
static priorityq *ready_queue;
static int reschedule;

void scheduler_init() {
	running = NULL;
	reschedule = TRUE;
	TRACE("\tready queue of size %d initialized", TASK_LIST_SIZE);
	ready_queue = priorityq_new(TASK_LIST_SIZE, NUM_PRIORITY);
}

volatile task_descriptor *scheduler_running() {
	return running;
}

void scheduler_ready(task_descriptor *td) {
	priorityq_push(ready_queue, td, td->priority);
}

int scheduler_empty() {
	return reschedule && PRIORITYQ_EMPTY(ready_queue);
}

task_descriptor *scheduler_get() {
	if (reschedule) {
		running = priorityq_pop(ready_queue);
	} else {
		reschedule = TRUE;
	}
	return (task_descriptor *) running;
}

void scheduler_killme() {
	free_user_memory(running->heap_base);
	td_free((task_descriptor *) running);
}

void scheduler_move2ready() {
	ASSERT(running, "no task running to move to ready");
	scheduler_ready((task_descriptor *) running);
}

void scheduler_runmenext() {
	reschedule = FALSE;
}
