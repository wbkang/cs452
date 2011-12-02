#pragma once

#include <util.h>

typedef struct ui_id {
	int tid;
	int id;
} ui_id;

void uiserver();
int uiserver_new();
ui_id uiserver_register();
void uiserver_effect(ui_id id, int flag, int color);
void uiserver_out(ui_id id, char *out);
void uiserver_move(ui_id id, int line, int col);
void uiserver_force_refresh(ui_id id);

#define uiserver_printf(id, ...) { \
	char buf[1024]; \
	ASSERT(MEMCHECK(), "shit!"); \
	sprintf(buf, __VA_ARGS__); \
	uiserver_out(id, buf); \
}

#define UIEFFECT_BRIGHT (1 << 0)
#define UIEFFECT_UNDERSCORE (1 << 1)
#define UIEFFECT_FGCOLOR (1 << 2)
