#pragma once

#include <util.h>
#include <task.h>

#define SYSCALL_CREATE 0
#define SYSCALL_MYTID 1
#define SYSCALL_MYPARENTTID 2
#define SYSCALL_PASS 3
#define SYSCALL_EXIT 4
#define SYSCALL_MALLOC 5
#define SYSCALL_SEND 6
#define SYSCALL_RECIEVE 7
#define SYSCALL_REPLY 8
#define SYSCALL_NAMESERVERTID 9

// asm
void asm_handle_swi();

void asm_switch_to_usermode(register_set *reg);

// system calls
int Create(int priority, func_t code);

int MyTid();

int MyParentsTid();

void Pass();

void Exit();

void* malloc(uint size);

int asm_Send(int tid, void* msg, char *reply, int lengths);

int Recieve(int *tid, void* msg, int msglen);

int Reply(int tid, void* reply, int replylen);

int NameServerTid();
