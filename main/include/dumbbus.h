#pragma once

#define NUM_SUBSCRIBERS 10

typedef void (*subscriber)(void*);
typedef struct dumbbus dumbbus;

struct dumbbus {
	int size;
	subscriber subscribers[NUM_SUBSCRIBERS];
};

dumbbus* dumbbus_new();
void dumbbus_register(dumbbus *dbus, subscriber s);
void dumbbus_unregister(dumbbus *dbus, subscriber s);
void dumbbus_dispatch(dumbbus *dbus, void*);
