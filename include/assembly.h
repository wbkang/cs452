#pragma once

#include <util.h>
#include <task.h>

#define SYSCALL_CREATE 0
#define SYSCALL_MYTID 1
#define SYSCALL_MYPARENTTID 2
#define SYSCALL_PASS 3
#define SYSCALL_EXIT 4
#define SYSCALL_MALLOC 5

// asm
void asm_handle_swi();
int asm_syscall(int reqid, void** args);
int asm_switch_to_usermode(register_set *reg);
