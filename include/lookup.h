#pragma once

#include <util.h>

typedef void* lookup_key;
typedef void* lookup_item;

typedef struct {
	int size;
	hash_fn hash;
	lookup_item arr[];
} lookup;

lookup *lookup_new(int size, hash_fn hash, lookup_item default_item);

static inline void lookup_put(lookup *this, lookup_key key, lookup_item item) {
	int h = this->hash(key);
	ASSERT(h < this->size, "bad hash function. h:%d", h);
	ASSERT(h < this->size, "invalid key. %x", key);
	this->arr[h] = item;
}

static inline lookup_item lookup_get(lookup *this, lookup_key key) {
	int h = this->hash(key);
	ASSERT(h < this->size, "bad hash function. h:%d", h);
	if (h < 0) {
		return NULL;
	}
	return this->arr[h];
}
