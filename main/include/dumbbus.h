#pragma once

#define NUM_SUBSCRIBERS 10

typedef void (*subscriber)(void* this, void* data);
typedef struct dumbbus dumbbus;

struct dumbbus {
	int size;
	subscriber subscribers[NUM_SUBSCRIBERS];
};

dumbbus *dumbbus_new();
void dumbbus_register(dumbbus *this, subscriber s);
void dumbbus_unregister(dumbbus *this, subscriber s);
void dumbbus_dispatch(dumbbus *this, void* that, void *data);
