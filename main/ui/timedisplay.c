#include <ui/timedisplay.h>
#include <syscall.h>
#include <uconst.h>
#include <server/uiserver.h>

timedisplay *timedisplay_new(int line, int col) {
	timedisplay *time = malloc(sizeof(timedisplay));
	time->id_ui = uiserver_register();
	time->line = line;
	time->col = col;
	return time;
}

void timedisplay_update(timedisplay* time, int ticks) {
	uiserver_move(time->id_ui, time->line, time->col);
	int ms = TICK2MS(ticks);
	uiserver_printf(time->id_ui, "Uptime: %d.%ds", ms / 1000, (ms / 100) % 10)
}
