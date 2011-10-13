#pragma once

#include <task.h>

void kernel_init();
int kernel_run();
inline int kernel_createtask(int priority, func_t code);
