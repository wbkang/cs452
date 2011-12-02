#include <ui/timedisplay.h>
#include <syscall.h>
#include <uconst.h>
#include <server/uiserver.h>

timedisplay *timedisplay_new(int line, int col) {
	timedisplay *time = malloc(sizeof(timedisplay));
	int tid_ui = WhoIs(NAME_UISERVER);
	time->tid_ui = tid_ui;
	time->id_ui = uiserver_register(tid_ui);
	time->line = line;
	time->col = col;
	return time;
}

void timedisplay_update(timedisplay* time, int ticks) {
	uiserver_move(time->tid_ui, time->id_ui, time->line, time->col);
	int ms = TICK2MS(ticks);
	uiserver_printf(time->tid_ui, time->id_ui, "Uptime: %d.%ds", ms / 1000, (ms / 100) % 10)
}
