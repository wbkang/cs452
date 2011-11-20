#pragma once

#include <console.h>

typedef struct {
	console *con;
	int line, col;
} logstrip;

logstrip *logstrip_new(console *con, int line, int col);

#define logstrip_printf(this, ...) { \
	console_move((this)->con, (this)->line, (this)->col); \
	console_erase_eol((this)->con); \
	console_printf((this)->con, __VA_ARGS__); \
	console_flush((this)->con); \
}
