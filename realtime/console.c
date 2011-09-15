/*
 * console.c - terminal related stuff
 *
 */

#include <bwio.h>
#include <console.h>

void console_putstr(char * str)
{
	bwputstr(COM2, str);
}

void console_clear()
{
	console_putstr("\033[2J");
}

void console_move(int line, int column)
{
	bwprintf(COM2, "\033[%d;%dH", line, column);
}
