#pragma once

#include <util.h>
#include <task.h>

void asm_handle_swi();

void asm_switch_to_usermode(register_set *reg);

int asm_Send(int tid, void* msg, char *reply, int lengths);
