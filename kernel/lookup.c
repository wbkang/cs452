#include <lookup.h>
#include <memory.h>
#include <util.h>

lookup *lookup_new(uint size, hash_fn hash, void* default_item) {
	lookup *this = qmalloc(sizeof(lookup) + sizeof(void*) * size);
	this->hash = hash;
	for (int i = 0; i < size; i++) {
		this->arr[i] = default_item;
	}
	return this;
}
