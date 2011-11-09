#include <lookup.h>
#include <memory.h>
#include <util.h>

lookup *lookup_new(int size, hash_fn hash, lookup_item default_item) {
	lookup *this = qmalloc(sizeof(lookup) + sizeof(lookup_item) * size);
	this->size = size;
	this->hash = hash;
	for (int i = 0; i < size; i++) {
		this->arr[i] = default_item;
	}
	return this;
}
