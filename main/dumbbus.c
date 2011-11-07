#include <dumbbus.h>
#include <util.h>

void dumbbus_init(dumbbus *dbus) {
	for (int i = 0; i < NUM_SUBSCRIBERS; i++) {
		dbus->subscribers[i] = NULL;
	}
}

void dumbbus_register(dumbbus *dbus, subscriber s) {
	for (int i = 0; i < NUM_SUBSCRIBERS; i++) {
		if (!dbus->subscribers[i]) {
			dbus->subscribers[i] = s;
			return;
		}
	}
	ASSERT(0, "no more room");
}

void dumbbus_unregister(dumbbus *dbus, subscriber s) {
	for (int i = 0; i < NUM_SUBSCRIBERS; i++) {
		if (dbus->subscribers[i] == s) {
			dbus->subscribers[i] = NULL;
			return;
		}
	}
	ASSERT(0, "couldn't find the subscriber %x", s);
}
void dumbbus_dispatch(dumbbus *dbus, void* data) {
	for (int i = 0; i < NUM_SUBSCRIBERS; i++) {
		if (dbus->subscribers[i]) {
			dbus->subscribers[i](data);
		}
	}
}
