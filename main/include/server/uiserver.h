#pragma once

#include <util.h>

void uiserver();
int uiserver_new();
int uiserver_register(int tid);
void uiserver_effect(int tid, int id, int flag, int color);
void uiserver_out(int tid, int id, char *out);
void uiserver_move(int tid, int id, int line, int col);
void uiserver_force_refresh(int tid);

#define uiserver_printf(tid, id, ...) { \
	char buf[1024]; \
	ASSERT(MEMCHECK(), "shit!"); \
	sprintf(buf, __VA_ARGS__); \
	uiserver_out(tid, id, buf); \
}

#define UIEFFECT_BRIGHT (1 << 0)
#define UIEFFECT_UNDERSCORE (1 << 1)
#define UIEFFECT_FGCOLOR (1 << 2)
