#pragma once

#include <util.h>
#include <stdio.h>

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

#define CONSOLE_CLEAR "\x1B[2J"
#define CONSOLE_ERASE_EOL "\x1B[K"
#define CONSOLE_EFFECT(effect) "\033[" TOSTRING(effect) "m"
#define CONSOLE_SHOWCURSOR "\x1B[?25h"
#define CONSOLE_HIDECURSOR "\x1B[?25l"
#define CONSOLE_SAVECURSOR "\x1B[s"
#define CONSOLE_UNSAVECURSOR "\x1B[u"

static inline int console_clear(char *buf) {
	return sprintf(buf, CONSOLE_CLEAR);
}

static inline int console_erase_eol(char *buf) {
	return sprintf(buf, CONSOLE_ERASE_EOL);
}

static inline int console_cursor_move(char *buf, int line, int column) {
	return sprintf(buf, "\x1B[%d;%dH", line, column);
}

static inline int console_cursor_left(char *buf, int count) {
	return sprintf(buf, "\x1B[%dD", count);
}

static inline int console_cursor_save(char *buf) {
	return sprintf(buf, CONSOLE_SAVECURSOR);
}

static inline int console_cursor_unsave(char *buf) {
	return sprintf(buf, CONSOLE_UNSAVECURSOR);
}

static inline int console_scroll(char *buf, uint start, uint end) {
	return sprintf(buf, "\x1B[%d;%dr", start, end);
}

static inline int console_fillrect(char *buf, char c, int x, int y, int width, int height) {
	char *orig_buf = buf;
	for (int i = y; i < y + height; i++) {
		buf += console_cursor_move(buf, i, x);
		for (int j = x; j < x + width; j++) {
			*buf++ = c;
		}
	}
	return buf - orig_buf;
}
