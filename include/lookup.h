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

static inline int lookup_goodkey(lookup *this, lookup_key key) {
	int h = this->hash(key);
	return 0 <= h && h < this->size;
}

static inline void lookup_put(lookup *this, lookup_key key, lookup_item item) {
	int h = this->hash(key);
	ASSERT(lookup_goodkey(this, key), "bad key/hash_fn! key: %x, hash_val: %d, hash_fn: %x", key, h, this->hash);
	this->arr[h] = item;
}

static inline lookup_item lookup_get(lookup *this, lookup_key key) {
	int h = this->hash(key);
	ASSERT(lookup_goodkey(this, key), "bad key/hash_fn! key: %x, hash_val: %d, hash_fn: %x", key, h, this->hash);
	return this->arr[h];
}
