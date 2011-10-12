#pragma once

#include <task.h>

void kernel_init();
void kernel_runloop();
inline int kernel_createtask(int priority, func_t code);
