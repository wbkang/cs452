#include <scheduler.h>
#include <priorityq.h>
#include <memory.h>

static task_descriptor *running;
static task_descriptor waiting4send;
static task_descriptor waiting4reply;
static priorityq *ready_queue;
static int reschedule;

void scheduler_init() {
	running = NULL;
	reschedule = TRUE;
	TD_CLOSE(&waiting4send);
	TD_CLOSE(&waiting4reply);
	TRACE("\tready queue of size %d initialized", TASK_LIST_SIZE);
	ready_queue = priorityq_new(TASK_LIST_SIZE, NUM_PRIORITY);
}

task_descriptor *scheduler_running() {
	return running;
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

void scheduler_ready(task_descriptor *td) {
	TD_REMOVE(td);
	td->state = TD_STATE_READY;
	priorityq_push(ready_queue, td, td->priority);
}

void scheduler_wait4send(task_descriptor *reciever) {
	TD_REMOVE(reciever);
	reciever->state = TD_STATE_WAITING4SEND;
	TD_PUSH(&waiting4send, reciever);
}

void scheduler_wait4recieve(task_descriptor *source, task_descriptor *td) {
	TD_REMOVE(td);
	td->state = TD_STATE_WAITING4RECIEVE;
	TD_PUSH(source, td);
}

void scheduler_wait4reply(task_descriptor *sender) {
	TD_REMOVE(sender);
	sender->state = TD_STATE_WAITING4REPLY;
	TD_PUSH(&waiting4reply, sender);
}
