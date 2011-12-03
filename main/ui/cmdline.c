#include <ui/cmdline.h>
#include <syscall.h>
#include <server/uiserver.h>

#define cmdline_putc(this, c) {\
	uiserver_move((this)->id_ui, (this)->line, (this)->col + (this)->cmdidx); \
	uiserver_printf((this)->id_ui, "%c", c); \
}

static void cmdline_movecursor(cmdline *this) {
	uiserver_movecursor(this->id_ui, this->line, this->col + this->cmdidx + 1);
}

cmdline *cmdline_new(int line, int col, cmdprocessor cmdproc) {
	cmdline *this = malloc(sizeof(cmdline));
	this->id_ui = uiserver_register();
	this->line = line;
	this->col = col;
	this->cmdidx = 0;
	this->cmdprocessor = cmdproc;
	cmdline_movecursor(this);
	return this;
}

void cmdline_handleinput(cmdline *this, char c, void* that) {
	if (this->cmdidx + 2 == LEN_CMD && c != '\b' && c != '\r') {
		cmdline_movecursor(this);
		return; // full, ignore
	}

	this->cmdbuf[this->cmdidx++] = c;
	this->cmdbuf[this->cmdidx] = '\0';

	switch (c) {
		case '\b':
			this->cmdidx--;
			if (this->cmdidx >= 1) {
				cmdline_putc(this, ' ');
				this->cmdidx -= 1;
			}
			break;
		case '\r':
			this->cmdbuf[--this->cmdidx] = '\0';
			this->cmdprocessor(that, this->cmdbuf, this->cmdidx);
			break;
		default:
			cmdline_putc(this, c);
			break;
	}

	cmdline_movecursor(this);
}

void cmdline_clear(cmdline *this) {
	this->cmdidx = 0;
	char fmt[10];
	sprintf(fmt, "%%%ds", LEN_CMD);
	uiserver_move(this->id_ui, this->line, this->col);
	uiserver_printf(this->id_ui, fmt, "");
}
