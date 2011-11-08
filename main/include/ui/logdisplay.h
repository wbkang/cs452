#pragma once
#include <console.h>
#include <stdio.h>

#define MAX_LOG_COL 128

typedef struct {
	int line, col;
	int totallines;
	int curline;
	int curcol;
	int topline;
	int rotation;
	struct console *con;
	char buf[][MAX_LOG_COL + 1];
} logdisplay;

logdisplay* logdisplay_new(console *con, int line, int col, int totallines, int rotation);
void logdisplay_puts(logdisplay *l, char* str);
void logdisplay_flushline(logdisplay *l);

#define logdisplay_printf(l, ...) { \
	char __logbuf[MAX_LOG_COL]; sprintf(__logbuf, __VA_ARGS__); logdisplay_puts(l, __logbuf);}



