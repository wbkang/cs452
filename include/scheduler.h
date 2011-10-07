#pragma once

#include <task.h>

void scheduler_init();

inline task_descriptor *scheduler_running();

inline int scheduler_empty();

task_descriptor *scheduler_get();

void scheduler_killme();

inline void scheduler_move2ready();

inline void scheduler_runmenext();

inline void scheduler_ready(task_descriptor *td);

inline void scheduler_wait4send(task_descriptor *receiver);

inline void scheduler_wait4receive(task_descriptor *source, task_descriptor *td);

inline void scheduler_wait4reply(task_descriptor *sender);
