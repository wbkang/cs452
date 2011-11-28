#pragma once

#include <util.h>

void kernel_init();
int kernel_run();
inline int kernel_createtask(int priority, func_t code);
void kernel_idleserver();
void handle_abort(int fp, int dabort);
