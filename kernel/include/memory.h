#pragma once

#include <util.h>

extern int _USER_MEM_START;
extern int _USER_MEM_END;

extern int _TextStart; // start of .text
extern int _TextEnd; // end of .text

struct _tag_task_descriptor;

int mem_init();
void mem_reset(); // reset kernel memory
void* kmalloc(uint size); // allocate kernel memory
void* umalloc(uint size); // allocate user memory
void* qmalloc(uint size); // branch allocation based on processor mode
void allocate_user_memory(struct _tag_task_descriptor *td);
void free_user_memory(struct _tag_task_descriptor *td);
