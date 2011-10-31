#include <scheduler.h>
#include <memory.h>
#include <task.h>

static task_descriptor *running;
static priorityq *ready_queue;
static int reschedule;
static task_descriptor *eventblocked[NUM_IRQS];

void scheduler_init() {
	running = NULL;
	ready_queue = priorityq_new(get_td_list_size(), NUM_PRIORITY);
	reschedule = TRUE;
	for (int irq = 0; irq < NUM_IRQS; irq++) {
		eventblocked[irq] = NULL;
	}
}

inline task_descriptor *scheduler_running() {
	return running;
}

inline int scheduler_empty() {
	return reschedule && priorityq_empty(ready_queue);
}

inline task_descriptor *scheduler_get() {
	if (reschedule) {
		running = priorityq_get(ready_queue);
		running->state = TD_STATE_RUNNING;
	} else {
		reschedule = TRUE;
	}
	return running;
}

inline void scheduler_ready(task_descriptor *td) {
	td->state = TD_STATE_READY;
	priorityq_put(ready_queue, td, td->priority);
}

inline void scheduler_readyme() {
	scheduler_ready(running);
}

inline void scheduler_runmenext() {
	reschedule = FALSE;
}

inline void scheduler_bindevent(int irq) {
	ASSERT(!eventblocked[irq], "irq %d bound to %d", irq, eventblocked[irq]->id);
	running->state = TD_STATE_WAITING4EVENT;
	eventblocked[irq] = running;
}

inline void scheduler_triggerevent(int irq) {
	task_descriptor *td = eventblocked[irq];
	ASSERT(td, "No task is event blocked on irq #%d", irq);
	eventblocked[irq] = NULL;
	scheduler_ready(td);
}

inline void scheduler_freeme() {
	while (!td_list_empty(running)) { // clearout receive blocked tasks
		task_descriptor *sender = td_list_pop(running);
		sender->registers.r[0] = -2;
		scheduler_ready(sender);
	}
	for (int irq = 0; irq < NUM_IRQS; irq++) { // remove me from eventblocked queue
		if (eventblocked[irq] == running) {
			VMEM(VIC1 + INTENCLR_OFFSET) = INT_MASK(irq);
			eventblocked[irq] = NULL;
		}
	}
	free_user_memory(running);
	td_free(running);
}
