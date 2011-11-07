#pragma once

#include <util.h>
#include <stdio.h>

#define CONSOLE_CLEAR "\x1B[2J"
#define CONSOLE_ERASE_EOL "\x1B[K"
#define CONSOLE_EFFECT(effect) "\033[" TOSTRING(effect) "m"
#define CONSOLE_SHOWCURSOR "\x1B[?25h"
#define CONSOLE_HIDECURSOR "\x1B[?25l"
#define CONSOLE_SAVECURSOR "\x1B[s"
#define CONSOLE_UNSAVECURSOR "\x1B[u"

#define EFFECT_RESET 0
#define EFFECT_REVERSE 7
#define EFFECT_FG_BLUE 34
#define EFFECT_FG_BLACK 30
#define EFFECT_FG_RED 31
#define EFFECT_BRIGHT 1
#define EFFECT_UNDERSCORE 4
#define EFFECT_FG_CYAN 36
#define EFFECT_FG_YELLOW 33
#define EFFECT_FG_GREEN 32
#define EFFECT_FG_WHITE 37

typedef struct console console;

#define CONSOLE_BUF_SIZE 4096

struct console {
	int tid_console;
	int buf_location;
	char buf[CONSOLE_BUF_SIZE+1];
};

#define CONSOLE_PRINTF(c, ...) { (c)->buf_location += sprintf((c)->buf + (c)->buf_location, __VA_ARGS__); \
	ASSERT((c)->buf_location <= CONSOLE_BUF_SIZE, "console buf overflow: %d", (c)->buf_location); }


void console_create(console *c, int tid);
void console_flush(console *c);

void console_effect(console *c, int effect);
void console_effect_reset(console *c);
void console_clear(console *console);
void console_erase_eol(console *c);
void console_move(console *c, int line, int column);
void console_cursor_left(console *c, int count);
void console_cursor_save(console *c) ;
void console_cursor_unsave(console *c);
void console_scroll(console *c, uint start, uint end);
