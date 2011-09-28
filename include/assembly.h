#pragma once

#include <util.h>

#define SYSCALL_CREATE 0
#define SYSCALL_MYTID 1
#define SYSCALL_MYPARENTTID 2
#define SYSCALL_PASS 3
#define SYSCALL_EXIT 4
#define SYSCALL_MALLOC 5

// asm
void asm_handle_swi();
int asm_syscall(int reqid, void** args);
void asm_switch_to_usermode(memptr sp, memptr entry_point);
void asm_usermode_wrapper(func_t entry_point);
