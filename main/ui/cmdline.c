#include <ui/cmdline.h>

#define CMDLINE_PUTC(cmd, c) {\
	console_move((cmd)->con, (cmd)->line, (cmd)->col + (cmd)->cmdidx); \
	console_printf((cmd)->con, "%c", c); \
	console_flush((cmd)->con); }

void cmdline_create(cmdline *cmd, int line, int col, console *con, cmdprocessor cmdproc, void *state) {
	cmd->cmdidx = 0;
	cmd->cmdprocessor = cmdproc;
	cmd->cmdprocstate = state;
	cmd->col = col;
	cmd->line = line;
	cmd->con = con;
}

void cmdline_handleinput(cmdline *cmd, char c) {
	if (cmd->cmdidx + 2 == LEN_CMD && c != '\b' && c != '\r') return; // full, ignore

	cmd->cmdbuf[cmd->cmdidx++] = c;
	cmd->cmdbuf[cmd->cmdidx] = '\0';

	console_move(cmd->con, cmd->line, cmd->col + cmd->cmdidx);
	console_printf(cmd->con, "%c", c);
	console_flush(cmd->con);

	switch(c) {
		case '\b':
			cmd->cmdidx--;
			if (cmd->cmdidx >= 1) {
				CMDLINE_PUTC(cmd, ' ');
				cmd->cmdidx -= 1;
			}
			break;
		case '\r':
			cmd->cmdbuf[--cmd->cmdidx] = '\0';
			cmd->cmdprocessor(cmd->cmdprocstate, cmd->cmdbuf, cmd->cmdidx);
			break;
		default:
			CMDLINE_PUTC(cmd, c);
			break;
	}
}

void cmdline_clear(cmdline *cmd) {
	cmd->cmdidx = 0;
	console_move(cmd->con, cmd->line, cmd->col);
	console_erase_eol(cmd->con);
	console_flush(cmd->con);
}
