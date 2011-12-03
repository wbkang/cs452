#pragma once

#include <util.h>

typedef struct ui_id {
	int tid;
	int id;
} ui_id;

void uiserver();
void uidrawserver();
int uiserver_new();
ui_id uiserver_register();
ui_id uiserver_register_blocking();
ui_id uiserver_register_with_tid(int tid);

void uiserver_effect(ui_id id, int flag, int color);
void uiserver_out(ui_id id, char *out);
void uiserver_move(ui_id id, int line, int col);
void uiserver_movecursor(ui_id id, int line, int col);
void uiserver_force_refresh(ui_id id);

#define uiserver_printf(id, ...) { \
	char __buf[1024]; \
	ASSERT(MEMCHECK(), "shit!"); \
	sprintf(__buf, __VA_ARGS__); \
	uiserver_out(id, __buf); \
}

#define UIEFFECT_BRIGHT (1 << 0)
#define UIEFFECT_UNDERSCORE (1 << 1)
#define UIEFFECT_FGCOLOR (1 << 2)
