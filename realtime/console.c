/*
 * console.c - terminal related stuff
 *
 */

#include <bwio.h>
#include <console.h>
#include <util.h>

#define CONSOLE_PORT COM2

void console_printf(char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	bwformat(CONSOLE_PORT, fmt, va);
	va_end(va);
}

void console_putstr(char * str)
{
	bwputstr(CONSOLE_PORT, str);
}

void console_putc(char c)
{
	bwputc(CONSOLE_PORT, c);
}

void console_fillrect(char c, int x, int y, int width, int height)
{
	for (int i = y; i < y + height; i++) {
		console_move(i, x);
		for (int j = x; j < x + width; j++) {
			console_putc(c);
		}
	}
}

void console_altfont(int use)
{
	console_putstr(use ? "\033)" : "\033(");
}

void console_effect(int effect)
{
	console_printf("\033[%dm", effect);
}

void console_clear()
{
	console_putstr("\033c");
	console_putstr("\033[2J");
}

void console_move(int line, int column)
{
	bwprintf(CONSOLE_PORT, "\033[%d;%dH", line, column);
}

void console_init()
{
	bwsetfifo(CONSOLE_PORT, OFF);
}
