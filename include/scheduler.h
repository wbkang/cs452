#pragma once

#include <task.h>

#define NUM_PRIORITY 32

void scheduler_init();

volatile task_descriptor *scheduler_running();

void scheduler_ready(task_descriptor *td);

int scheduler_empty();

task_descriptor *scheduler_get();

void scheduler_killme();

void scheduler_move2ready();

void scheduler_runmenext();
