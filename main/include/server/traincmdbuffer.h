#pragma once

#include <uconst.h>

int traincmdbuffer_new();
int traincmdbuffer_put(int tid, traincmdname name, int arg1, int arg2);
int traincmdbuffer_get(int tid, traincmd *cmd);
