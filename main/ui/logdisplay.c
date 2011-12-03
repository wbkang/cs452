#include <syscall.h>
#include <ui/logdisplay.h>
#include <server/uiserver.h>
#include <stdio.h>
#include <string.h>
#include <console.h>

#define ROUND_ROBIN_PREFIX "> "

static void logdisplay_put_title(logdisplay* this) {
	uiserver_move(this->id_ui, this->line - 1, this->col);
	uiserver_effect(this->id_ui, UIEFFECT_BRIGHT, 0);
	char fmt[10];
	sprintf(fmt, "%%-%ds", this->totalcols);
	uiserver_printf(this->id_ui, fmt, this->title);
	uiserver_effect(this->id_ui, 0, 0);
}

logdisplay *logdisplay_new(int line, int col, int totallines, int totalcols, int rotation, char *title) {
	logdisplay *this = malloc(sizeof(logdisplay) + sizeof(char*) * totallines);
	this->id_ui = uiserver_register();
	this->line = line + 1; // for the title
	this->col = col;
	this->totallines = totallines - 1; // for the title
	this->totalcols = totalcols;
	this->title = title;
	if (rotation == ROUNDROBIN) this->totalcols -= strlen(ROUND_ROBIN_PREFIX); // accounting for the starting arrow
	this->curcol = 0;
	this->curline = 0;
	this->topline = 0;

	for (int i = 0; i < totallines; i++) {
		this->buf[i] = malloc(totalcols + 1);
		this->buf[i][0] = '\0';
	}

	this->rotation = rotation;
	logdisplay_put_title(this);
	return this;
}

void logdisplay_puts(logdisplay *this, char *str) {
	int curline = this->curline;
	int curcol = this->curcol;
	int totalcols = this->totalcols;
	int len =  strlen(str);
	int copycount = min(totalcols - curcol, len);

	strncpy(this->buf[curline] + curcol, str, copycount);

	this->curcol += copycount;
	this->buf[curline][this->curcol] = '\0';
}

void logdisplay_flushline(logdisplay *this) {
	this->curcol = 0;
	char fmtbuf[10];
	if (this->rotation == ROUNDROBIN) {
		int last_line = (this->totallines + this->curline - 1) % this->totallines;
		uiserver_move(this->id_ui, this->line + last_line, this->col);
		uiserver_printf(this->id_ui, " ", this->buf[last_line]);
		uiserver_move(this->id_ui, this->line + this->curline, this->col);
		sprintf(fmtbuf, ROUND_ROBIN_PREFIX "%%-%ds", this->totalcols);
		uiserver_printf(this->id_ui, fmtbuf, this->buf[this->curline]);
		this->curline++;
		this->curline = this->curline % this->totallines;
	} else if (this->rotation == SCROLLING) {
		sprintf(fmtbuf, "%%-%ds", this->totalcols);
		for (int i = 0, cl = this->topline % this->totallines; i < this->totallines; i++) {
			uiserver_move(this->id_ui, this->line + i, this->col);
			uiserver_printf(this->id_ui, fmtbuf, this->buf[cl]);
			cl++;
			cl = cl % this->totallines;
		}

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
