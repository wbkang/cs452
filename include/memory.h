#pragma once

#include <util.h>

// the size of user memory in bytes (64 KB)
#define STACK_SIZE 65536

void mem_init();

void mem_reset();

void* kmalloc(uint size);

void* umalloc(uint size);

void* allocate_user_memory();

void free_user_memory(memptr heapbase);
