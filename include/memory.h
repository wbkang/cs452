#pragma once

#include <util.h>

void mem_init();

void mem_reset();

void* kmalloc(uint size);

void* umalloc(uint size);
