#pragma once

#include <util.h>

int Create(int priority, func_t code);

int MyTid();

int MyParentsTid();

void Pass();

void Exit();

void* malloc(unsigned int size);
