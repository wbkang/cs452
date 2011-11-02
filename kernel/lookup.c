#include <lookup.h>
#include <memory.h>
#include <util.h>

lookup* lookup_new(uint size, hash_func f, void* default_value) {
	lookup *l = qmalloc(sizeof(lookup) + size * sizeof(void*));
	l->hash = f;
	for (int i = 0; i < size; i++) {
		l->ary[i] = default_value;
	}
	return l;
}
void lookup_put(lookup *l, void *key, void *item) {
	l->ary[l->hash(key)] = item;

}

void* lookup_get(lookup *l, void *key) {
	return l->ary[l->hash(key)];
}
