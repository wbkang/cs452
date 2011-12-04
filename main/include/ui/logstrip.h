#pragma once

#include <console.h>
#include <server/uiserver.h>
#include <string.h>
#include <util.h>

typedef struct {
	int tid_ui;
	ui_id id_ui;
	int width;
	int line, col;
} logstrip;

logstrip *logstrip_new(int line, int col, int width);

#define logstrip_printf(this, ...) { \
	char __fmt[10], __pbuf[256], __buf[256]; ASSERT(MEMCHECK(), "stack overflow"); \
	sprintf(__fmt, "%%-%ds", (this)->width); \
	sprintf(__buf, __VA_ARGS__); \
	sprintf(__pbuf, __fmt, __buf); \
	uiserver_move((this)->id_ui, (this)->line, (this)->col); \
	uiserver_out((this)->id_ui, __pbuf); \
}
