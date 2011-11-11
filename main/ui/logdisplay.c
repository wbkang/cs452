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

logdisplay *logdisplay_new(console *con, int line, int col, int totallines, int rotation) {
	logdisplay *this = malloc(sizeof(logdisplay) + MAX_LOG_COL * totallines);
	this->con = con;
	this->line = line;
	this->col = col;
	this->totallines = totallines;
	this->curcol = 0;
	this->curline = 0;
	this->topline = 0;

	for (int i = 0; i < totallines; i++) {
		this->buf[i][0] = '\0';
	}

	this->rotation = rotation;
	return this;
}

void logdisplay_puts(logdisplay *this, char *str) {
	ASSERT(strlen(str) + this->curcol < MAX_LOG_COL, "logdisplay overflow trying to put '%s'", str);
	this->curcol += sprintf(this->buf[this->curline] + this->curcol, "%s", str);
}

void logdisplay_flushline(logdisplay *this) {
	this->curcol = 0;
	if (this->rotation == ROUNDROBIN) {
		int last_line = (this->totallines + this->curline - 1) % this->totallines;
		console_move(this->con, this->line + last_line, this->col);
		console_printf(this->con, " ", this->buf[last_line]);
		console_move(this->con, this->line + this->curline, this->col);
		console_erase_eol(this->con);
		console_printf(this->con, "> %s", this->buf[this->curline]);
		console_flush(this->con);
		this->curline++;
		this->curline = this->curline % this->totallines;
	} else if (this->rotation == SCROLLING) {
		for (int i = 0, cl = this->topline % this->totallines; i < this->totallines; i++) {
			console_move(this->con, this->line + i, this->col);
			console_erase_eol(this->con);
			console_printf(this->con, "%s", this->buf[cl]);
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
