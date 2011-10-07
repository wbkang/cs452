#pragma once

#include <task.h>

void kernel_init();

void handle_swi(register_set *reg);

void kernel_runloop();

inline int kernel_createtask(int priority, func_t code);

inline int kernel_mytid();

inline int kernel_myparenttid();

inline void kernel_passtask();

inline void kernel_exittask();

inline void *kernel_malloc(uint size);

inline void kernel_exit();

inline int kernel_send(int tid, void* msg, int msglen, void* reply, int replylen);

inline int kernel_receive(int *tid, void* msg, int msglen);

inline int kernel_reply(int tid, void* reply, int replylen);
