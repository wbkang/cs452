#pragma once

#include <util.h>
#include <stdio.h>

#define CONSOLE_CLEAR "\x1B[2J"
#define CONSOLE_ERASE_SOL "\x1B[1K"
#define CONSOLE_ERASE_EOL "\x1B[K"
#define CONSOLE_EFFECT(effect) "\033[" TOSTRING(effect) "m"
#define CONSOLE_SHOWCURSOR "\x1B[?25h"
#define CONSOLE_HIDECURSOR "\x1B[?25l"
#define CONSOLE_SAVECURSOR "\x1B[s"
#define CONSOLE_UNSAVECURSOR "\x1B[u"

#define EFFECT_FG_BLACK 30
#define EFFECT_FG_RED 31
#define EFFECT_FG_GREEN 32
#define EFFECT_FG_YELLOW 33
#define EFFECT_FG_BLUE 34
#define EFFECT_FG_CYAN 36
#define EFFECT_FG_MAGENTA 35
#define EFFECT_FG_WHITE 37

#define EFFECT_RESET 0
#define EFFECT_REVERSE 7
#define EFFECT_BRIGHT 1
#define EFFECT_UNDERSCORE 4

typedef struct console console;

#define CONSOLE_BUF_SIZE 4096

struct console {
	int tid_console;
	int buf_location;
	char buf[CONSOLE_BUF_SIZE + 1];
};

#define console_printf(c, ...) { \
	(c)->buf_location += sprintf((c)->buf + (c)->buf_location, __VA_ARGS__); \
	ASSERT((c)->buf_location <= CONSOLE_BUF_SIZE, "console buf overflow: %d", (c)->buf_location); \
}

console *console_new(int channel);
void console_flush(console *this);
void console_flushcom(console *this);
void console_hidecursor(console *this);
void console_effect(console *this, int effect);
void console_effect_reset(console *this);
void console_clear(console *this);
void console_erase_bol(console *this);
void console_erase_eol(console *this);
void console_move(console *this, int line, int column);
void console_cursor_left(console *this, int count);
void console_cursor_save(console *this) ;
void console_cursor_unsave(console *this);
void console_scroll(console *this, uint start, uint end);
