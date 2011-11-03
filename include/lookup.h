#pragma once

#include <util.h>

typedef uint (*hash_fn)(void* item);

typedef struct {
	hash_fn hash;
	void* arr[];
} lookup;

lookup *lookup_new(uint size, hash_fn hash, void* default_item);

static inline void lookup_put(lookup *this, void* key, void* item) {
	this->arr[this->hash(key)] = item;
}

static inline void* lookup_get(lookup *this, void* key) {
	return this->arr[this->hash(key)];
}
