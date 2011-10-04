#pragma once

#include <task.h>

void kernel_init();

void handle_swi(register_set *reg);

int kernel_createtask(int priority, func_t code);

int kernel_mytid();

int kernel_myparenttid();

void kernel_passtask();

void kernel_exittask();

void *kernel_malloc(uint size);

void kernel_runloop();

int kernel_send(int tid, void* msg, int msglen, void* reply, int replylen);

int kernel_recieve(int *tid, void* msg, int msglen);

int kernel_reply(int tid, void* reply, int replylen);
