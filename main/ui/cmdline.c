#include <ui/cmdline.h>
#include <syscall.h>

#define cmdline_putc(this, c) {\
	console_move((this)->con, (this)->line, (this)->col + (this)->cmdidx); \
	console_printf((this)->con, "%c", c); \
	console_flush((this)->con); }

static void cmdline_movecursor(cmdline *this) {
	console_cursor_unsave(this->con);
	console_move(this->con, this->line, this->col + this->cmdidx + 1);
	console_cursor_save(this->con);
	console_flush(this->con);
}

cmdline *cmdline_new(console *con, int line, int col, cmdprocessor cmdproc, void *state) {
	cmdline *this = malloc(sizeof(cmdline));
	this->con = con;
	this->line = line;
	this->col = col;
	this->cmdidx = 0;
	this->cmdprocessor = cmdproc;
	this->cmdprocstate = state;
	cmdline_movecursor(this);
	return this;
}

void cmdline_handleinput(cmdline *this, char c) {
	if (this->cmdidx + 2 == LEN_CMD && c != '\b' && c != '\r') {
		cmdline_movecursor(this);
		return; // full, ignore
	}

	this->cmdbuf[this->cmdidx++] = c;
	this->cmdbuf[this->cmdidx] = '\0';

	switch(c) {
		case '\b':
			this->cmdidx--;
			if (this->cmdidx >= 1) {
				cmdline_putc(this, ' ');
				this->cmdidx -= 1;
			}
			break;
		case '\r':
			this->cmdbuf[--this->cmdidx] = '\0';
			this->cmdprocessor(this->cmdprocstate, this->cmdbuf, this->cmdidx);
			break;
		default:
			cmdline_putc(this, c);
			break;
	}

	cmdline_movecursor(this);
}

void cmdline_clear(cmdline *this) {
	this->cmdidx = 0;
	console_move(this->con, this->line, this->col);
	console_erase_eol(this->con);
	console_flush(this->con);
}
