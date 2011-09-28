#pragma once

#include <task.h>

#define NUM_PRIORITY 32

void kernel_init();

void handle_swi(register_set *reg, int req_no);

/*
 * Create allocates and initializes a task descriptor, using the given priority,
 * and the given function pointer as a pointer to the entry point of executable
 * code, essentially a function with no arguments and no return value. When
 * create returns the task descriptor has all the state needed to run the task,
 * the task's stack has been suitably initialize, and the task has been entered
 * into its ready queue so that it will run the next time it is scheduled.
 *
 * Returns
 * 		tid - the positive integer task id of the newly created task. The task
 * 			id must be unique, in the sense that no task has, will have or has
 * 			had the same task id.
 * 		-1 - if the priority is invalid.
 * 		-2 - if the kernel is out of task descriptors.
 */
int kernel_createtask(int priority, func_t code);

/*
 * MyTid returns the task id of the calling task.
 *
 * Returns
 * 		tid - the positive integer task id of the task that calls it.
 * 		Errors should be impossible!
 */
int kernel_mytid();

/*
 * MyParentTid returns the task id of the task that created the calling task.
 *
 * 	This will be problematic only if the task has exited or been destroyed, in
 * 	which case the return value is implementation-dependent.
 *
 * 	Returns
 * 		tid - the task id of the task that created the calling task.
 * 		The return value is implementation-dependent if the parent has exited,
 * 		has been destroyed or is in the process of being destroyed.
 */
int kernel_myparenttid();

/*
 * Pass causes a task to stop executing. The task is moved to the end of its
 * priority queue, and will resume executing when next scheduled.
 */
void kernel_passtask();

/*
 * Exit causes a task to cease execution permanently. It is removed from all
 * priority queues, send queues, receive queues and awaitEvent queues. Resources
 * owned by the task, primarily its memory and task descriptor are not
 * reclaimed.
 */

void kernel_exittask();

void kernel_runloop();

volatile task_descriptor* kernel_td_current();

void kernel_driver(func_t code);
