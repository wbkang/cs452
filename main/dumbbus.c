#include <dumbbus.h>
#include <util.h>
#include <syscall.h>

dumbbus *dumbbus_new() {
	dumbbus *this = malloc(sizeof(dumbbus));
	this->size = 0;
	return this;
}

void dumbbus_register(dumbbus *this, subscriber s) {
	ASSERT(this->size < NUM_SUBSCRIBERS, "overflow. cursize: %d,item: %x", this->size, s);
	this->subscribers[this->size++] = s;
}

void dumbbus_unregister(dumbbus *this, subscriber s) {
	for (int i = 0; i < this->size; i++) {
		if (this->subscribers[i] == s) {
			this->subscribers[i] = this->subscribers[--this->size];
			return;
		}
	}
	ASSERT(0, "couldn't find the subscriber %x", s);
}

void dumbbus_dispatch(dumbbus *this, void* data) {
	for (int i = 0; i < this->size; i++) {
		this->subscribers[i](data);
	}
}
