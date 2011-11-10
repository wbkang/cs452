#include <dumbbus.h>
#include <util.h>
#include <syscall.h>

dumbbus* dumbbus_new() {
	dumbbus *dbus = malloc(sizeof(dumbbus));
	dbus->size = 0;
	return dbus;
}

void dumbbus_register(dumbbus *dbus, subscriber s) {
	ASSERT(dbus->size < NUM_SUBSCRIBERS, "overflow. cursize: %d,item: %x", dbus->size, s);
	dbus->subscribers[dbus->size++] = s;
}

void dumbbus_unregister(dumbbus *dbus, subscriber s) {
	for (int i = 0; i < dbus->size; i++) {
		if (dbus->subscribers[i] == s) {
			dbus->subscribers[i] = dbus->subscribers[--dbus->size];
			return;
		}
	}
	ASSERT(0, "couldn't find the subscriber %x", s);
}
void dumbbus_dispatch(dumbbus *dbus, void* data) {
	for (int i = 0; i < dbus->size; i++) {
		dbus->subscribers[i](data);
	}
}
