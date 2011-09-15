/*
 * console.c - terminal related stuff
 *
 */

#include <bwio.h>
#include <console.h>
#include <util.h>

#define CONSOLE_PORT COM2

void console_putstr(char * str)
{
	bwputstr(CONSOLE_PORT, str);
}

void console_clear()
{
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
