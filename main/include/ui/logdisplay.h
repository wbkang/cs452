#pragma once
#include <console.h>
#include <stdio.h>

#define MAX_LOG_COL 256

typedef struct {
	int line, col;
	int totallines;
	int totalcols;
	int curline;
	int curcol;
	int topline;
	enum { ROUNDROBIN, SCROLLING } rotation;
	struct console *con;
	char *title;
	char *buf[];
} logdisplay;

logdisplay *logdisplay_new(console *con, int line, int col, int totallines, int totalcols, int rotation, char *title);
void logdisplay_puts(logdisplay *this, char* str);
void logdisplay_flushline(logdisplay *this);

#define logdisplay_printf(this, ...) { \
	char __logbuf[MAX_LOG_COL]; \
	sprintf(__logbuf, __VA_ARGS__); \
	logdisplay_puts(this, __logbuf); \
}
