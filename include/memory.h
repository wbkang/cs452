#pragma once

#include <util.h>

struct _tag_task_descriptor;

void mem_init(uint task_list_size);

void mem_reset(); // reset kernel memory

void* kmalloc(uint size); // allocate kernel memory

void* umalloc(uint size); // allocate user memory

void* qmalloc(uint size); // branch allocation based on processor mode

void allocate_user_memory(struct _tag_task_descriptor *td);

void free_user_memory(struct _tag_task_descriptor *td);
