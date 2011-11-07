#pragma once
#include <console.h>

typedef struct {
	int line, col;
	console *con;
} logstrip;

logstrip logstrip_create(int line, int col, console *con);

#define logstrip_printf(l, ...) { \
	console_move((l).con, (l).line, (l).col); console_erase_eol((l).con); \
	console_printf((l).con, __VA_ARGS__); \
	console_flush((l).con); }
