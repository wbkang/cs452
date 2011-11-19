#include <ui/timedisplay.h>
#include <syscall.h>
#include <uconst.h>

timedisplay *timedisplay_new(console *con, int line, int col) {
	timedisplay *td = malloc(sizeof(timedisplay));
	td->con = con;
	td->line = line;
	td->col = col;
	return td;
}

void timedisplay_update(timedisplay* time, int ticks) {
	console_move(time->con, time->line, time->col);
	int ms = TICK2MS(ticks);
	console_printf(time->con, "%d.%d%-10s", ms / 1000, (ms / 100) % 10, "s");
	console_flush(time->con);
}
