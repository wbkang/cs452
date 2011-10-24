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
#define EFFECT_FG_YELLOW 33
#define EFFECT_FG_GREEN 32
#define EFFECT_FG_WHITE 37

#define CONSOLE_CLEAR "\033c\033[2J"
#define CONSOLE_EFFECT(effect) "\033[" TOSTRING(effect) "m"
#define CONSOLE_SHOWCURSOR "\033[?25h"
#define CONSOLE_HIDECURSOR "\033[?25l"
#define CONSOLE_SAVECURSOR "\0337"
#define CONSOLE_LOADCURSOR "\0338"

static inline int console_move(char * buf, int line, int column)
{
	return sprintf(buf, "\033[%d;%df", line, column);
}

static inline int console_fillrect(char * buf, char c, int x, int y, int width, int height)
{
	char * orig_buf = buf;

	for (int i = y; i < y + height; i++) {
		buf += console_move(buf, i, x);
		for (int j = x; j < x + width; j++) {
			*buf++ = c;
		}
	}

	return buf - orig_buf;
}
