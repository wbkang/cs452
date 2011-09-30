#pragma once

#include <task.h>

#define NUM_PRIORITY 32

void scheduler_init();

void scheduler_ready(task_descriptor *td);

int scheduler_empty();

task_descriptor *scheduler_get();
