#pragma once

#include <util.h>

typedef void* lookup_key;
typedef void* lookup_item;

typedef struct {
	uint size;
	hash_fn hash;
	lookup_item arr[];
} lookup;

lookup *lookup_new(uint size, hash_fn hash, lookup_item default_item);

static inline void lookup_put(lookup *this, lookup_key key, lookup_item item) {
	uint h = this->hash(key);
	ASSERT(h < this->size, "bad hash function");
	this->arr[h] = item;
}

static inline lookup_item lookup_get(lookup *this, lookup_key key) {
	uint h = this->hash(key);
	ASSERT(h < this->size, "bad hash function");
	return this->arr[h];
}
