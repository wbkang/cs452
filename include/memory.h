#pragma once

#include <util.h>

struct _tag_task_descriptor;

// also look at the variables in orex.ld
#define USER_MEM_START	0x300000
#define USER_MEM_END	0x1900000
// size of user memory in bytes (64 KB)
#define STACK_SIZE 65536

extern int _TextStart;
extern int _TextEnd;

void mem_init(uint task_list_size);

void mem_reset(); // reset kernel memory

void* kmalloc(uint size); // allocate kernel memory

void* umalloc(uint size); // allocate user memory

void* qmalloc(uint size); // branch allocation based on processor mode

void allocate_user_memory(struct _tag_task_descriptor *td);

void free_user_memory(struct _tag_task_descriptor *td);
