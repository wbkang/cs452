#pragma once

int Create(int priority, void (*code)());
int MyTid();
int MyParentsTid();
void Pass();
void Exit();
void* malloc(unsigned int size);
