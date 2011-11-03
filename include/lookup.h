#pragma once

#include <util.h>

typedef void* lookup_key;
typedef void* lookup_item;

typedef struct {
	hash_fn hash;
	lookup_item arr[];
} lookup;

lookup *lookup_new(uint size, hash_fn hash, lookup_item default_item);

static inline void lookup_put(lookup *this, lookup_key key, lookup_item item) {
	this->arr[this->hash(key)] = item;
}

static inline lookup_item lookup_get(lookup *this, lookup_key key) {
	return this->arr[this->hash(key)];
}
