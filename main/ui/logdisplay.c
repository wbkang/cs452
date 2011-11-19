#include <syscall.h>
#include <ui/logdisplay.h>
#include <stdio.h>
#include <string.h>
#include <console.h>

#define ROUND_ROBIN_PREFIX "> "

logdisplay *logdisplay_new(console *con, int line, int col, int totallines, int totalcols, int rotation) {
	logdisplay *this = malloc(sizeof(logdisplay) + sizeof(char*) * totallines);
	this->con = con;
	this->line = line;
	this->col = col;
	this->totallines = totallines;
	this->totalcols = totalcols;
	if (rotation == ROUNDROBIN) this->totalcols -= strlen(ROUND_ROBIN_PREFIX); // accounting for the starting arrow
	this->curcol = 0;
	this->curline = 0;
	this->topline = 0;

	for (int i = 0; i < totallines; i++) {
		this->buf[i] = malloc(totalcols + 1);
		this->buf[i][0] = '\0';
	}

	this->rotation = rotation;
	return this;
}

void logdisplay_puts(logdisplay *this, char *str) {
	int curline = this->curline;
	int curcol = this->curcol;
	int totalcols = this->totalcols;
	int len =  strlen(str);
	int copycount = MIN(totalcols - curcol, len);

	strncpy(this->buf[curline] + curcol, str, copycount);

	this->curcol += copycount;
	this->buf[curline][this->curcol] = '\0';
}

void logdisplay_flushline(logdisplay *this) {
	this->curcol = 0;
	char fmtbuf[10];
	if (this->rotation == ROUNDROBIN) {
		int last_line = (this->totallines + this->curline - 1) % this->totallines;
		console_move(this->con, this->line + last_line, this->col);
		console_printf(this->con, " ", this->buf[last_line]);
		console_move(this->con, this->line + this->curline, this->col);
		sprintf(fmtbuf, ROUND_ROBIN_PREFIX "%%-%ds", this->totalcols);
		console_printf(this->con, fmtbuf, this->buf[this->curline]);
		console_flush(this->con);
		this->curline++;
		this->curline = this->curline % this->totallines;
	} else if (this->rotation == SCROLLING) {
		sprintf(fmtbuf, "%%-%ds", this->totalcols);
		for (int i = 0, cl = this->topline % this->totallines; i < this->totallines; i++) {
			console_move(this->con, this->line + i, this->col);
			console_printf(this->con, fmtbuf, this->buf[cl]);
			cl++;
			cl = cl % this->totallines;
		}
		console_flush(this->con);

		this->curline++;
		this->curline = this->curline % this->totallines;

		if (this->topline || (this->topline == 0 && this->curline == 0)) {
			this->topline++;
			if (this->topline < 0) this->topline = 1;
		}
	} else {
		ASSERT(0, "invalid rotation mode: %d", this->rotation);
	}
}
