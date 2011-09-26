#pragma once

#define SYSCALL_CREATE 0
#define SYSCALL_MYTID 1
#define SYSCALL_MYPARENTTID 2
#define SYSCALL_PASS 3
#define SYSCALL_EXIT 4



// asm
void asm_handle_swi();
int asm_syscall(int reqid, void** args);
