#pragma once

#include <util.h>
#include <task.h>

// also look at the variables in orex.ld
#define USER_MEM_START	0x300000
#define USER_MEM_END	0x1900000
// size of user memory in bytes (64 KB)
#define STACK_SIZE 65536
// this is calculated at compile time.
#define TASK_LIST_SIZE ((USER_MEM_END - USER_MEM_START) / 65536)

#define BYTES2WORDS(x) ((x) >> 2)

#define NEXTHIGHESTWORD(x) BYTES2WORDS((x) + 3)

extern int _TextStart;
extern int _TextEnd;

void mem_init();

void mem_reset();

void* kmalloc(uint size); // allocate kernel memory

void* umalloc(uint size); // allocate user memory

void* qmalloc(uint size); // branch allocation based on processor mode

void allocate_user_memory(task_descriptor *td);

void free_user_memory(task_descriptor *td);
