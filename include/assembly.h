#pragma once

#include <util.h>
#include <task.h>

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

int Receive(int *tid, void* msg, int msglen);

int Reply(int tid, void* reply, int replylen);

int NameServerTid();
