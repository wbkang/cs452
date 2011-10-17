#pragma once

#include <task.h>

void scheduler_init();
inline task_descriptor *scheduler_running();
inline int scheduler_empty();
inline task_descriptor *scheduler_get();
inline void scheduler_readyme();
inline void scheduler_runmenext();
inline void scheduler_ready(task_descriptor *td);
inline void scheduler_wait4send(task_descriptor *receiver);
inline void scheduler_wait4receive(task_descriptor *receiver, task_descriptor *sender);
inline void scheduler_wait4reply(task_descriptor *sender);
inline void scheduler_bindevent(int irq);
inline void scheduler_triggerevent(int irq);
inline void scheduler_freeme();
