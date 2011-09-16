/*
 * console.c - terminal related stuff
 *
 */

#include <bwio.h>
#include <console.h>
#include <util.h>
#include <rawio.h>

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
//raw_putcc(COM2, 0x1c);
	bwputstr(CONSOLE_PORT, str);
}

void console_clear()
{
//raw_putcc(COM2, 0x1b);
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
