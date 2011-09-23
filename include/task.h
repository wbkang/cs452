#pragma once

#include <hardware.h>

typedef struct _tag_register_set {
	int registers[15]; // r0-r12, sp and lr
} * register_set;

typedef struct _tag_task_descriptor {
	int registers[15]; // r0-r12, sp and lr
} * task_descriptor;
