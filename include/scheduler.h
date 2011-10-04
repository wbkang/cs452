#pragma once

#include <task.h>

#define NUM_PRIORITY 32
#define MAX_PRIORITY (NUM_PRIORITY - 1)
#define MIN_PRIORITY 0

void scheduler_init();

task_descriptor *scheduler_running();

int scheduler_empty();

task_descriptor *scheduler_get();

void scheduler_killme();

void scheduler_move2ready();

void scheduler_runmenext();

void scheduler_ready(task_descriptor *td);

void scheduler_wait4send(task_descriptor *reciever);

void scheduler_wait4recieve(task_descriptor *source, task_descriptor *td);

void scheduler_wait4reply(task_descriptor *sender);
