#pragma once

#include <util.h>

// also look at the variables in orex.ld
#define USER_MEM_START	0x300000
#define USER_MEM_END	0x1900000
// the size of user memory in bytes (64 KB)
#define STACK_SIZE 65536
// this is calculated in compile time.
#define TASK_LIST_SIZE ((USER_MEM_END - USER_MEM_START) / 65536)

extern int _TextStart;
extern int _TextEnd;

void mem_init();

void mem_reset();

void* kmalloc(uint size); // allocate kernel memory

void* umalloc(uint size); // allocate user memory

void* qmalloc(uint size); // branch allocation based on processor mode

void* allocate_user_memory();

void free_user_memory(memptr heapbase);
