#pragma once

#include <util.h>
#include <task.h>

void asm_handle_swi();
uint asm_switch_to_usermode(register_set *reg);
int asm_Send(int tid, void* msg, char *reply, int lengths);
void asm_handle_abort();
void asm_handle_dabort();
