#pragma once

#include <util.h>

typedef uint (*hash_func)(void* item);
typedef struct {
	hash_func hash;
	void *ary[];
} lookup;

lookup* lookup_new(uint size, hash_func f, void *default_value);
void 	lookup_put(lookup *l, void *key, void *item);
void*	lookup_get(lookup *l, void *key);
