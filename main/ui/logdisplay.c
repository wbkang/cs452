#include <syscall.h>
#include <ui/logdisplay.h>
#include <stdio.h>
#include <string.h>
#include <console.h>
//typedef struct {
//	int line, col;
//	int totallines;
//	int curline;
//	int rotation;
//	struct console *con;
//	char buf[][MAX_LOG_COL];
//} logdisplay;


logdisplay* logdisplay_new(console *con, int line, int col, int totallines, int rotation) {
	logdisplay *l = malloc(sizeof(logdisplay) + MAX_LOG_COL * totallines);
	l->con = con;
	l->line = line;
	l->col = col;
	l->totallines = totallines;
	l->curcol = 0;
	l->curline = 0;
	l->topline = 0;

	for (int i = 0; i < totallines; i++) {
		l->buf[i][0] = '\0';
	}

	l->rotation = rotation;
	return l;
}

void logdisplay_puts(logdisplay *l, char* str) {
	ASSERT(strlen(str) + l->curcol < MAX_LOG_COL, "logdisplay overflow trying to put '%s'", str);
	l->curcol += sprintf(l->buf[l->curline] + l->curcol, "%s", str);
}

void logdisplay_flushline(logdisplay *l) {
	l->curcol = 0;
	if (l->rotation) {
		console_move(l->con, l->line + l->curline, l->col);
		console_erase_eol(l->con);
		console_printf(l->con, "%s", l->buf[l->curline]);
		console_flush(l->con);
		l->curline++;
		l->curline = l->curline % l->totallines;
	} else {
		for (int i = 0, cl = l->topline % l->totallines; i < l->totallines; i++) {
			console_move(l->con, l->line + i, l->col);
			console_erase_eol(l->con);
			console_printf(l->con, "%s", l->buf[cl]);
			cl++;
			cl = cl % l->totallines;
		}
		console_flush(l->con);

		l->curline++;
		l->curline = l->curline % l->totallines;

		if (l->topline || (l->topline == 0 && l->curline == 0)) {
			l->topline++;
			if (l->topline < 0) l->topline = 1;
		}
	}
}
