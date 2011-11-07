#pragma once

#define MAX_SUBSCRIBER 10

typedef void (*subscriber(void*));
typedef struct dumbbus dumbbus;

struct dumbbus
{
	subscriber *subscribers[MAX_SUBSCRIBER];
};

void dumbbus_init(dumbbus *dbus);
void dumbbus_register(dumbbus *dbus, subscriber s);
void dumbbus_unregister(dumbbus *dbus, subscriber s);
void dumbbus_dispatch(dumbbus *dbus, void*);

