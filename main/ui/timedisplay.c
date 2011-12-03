#include <ui/timedisplay.h>
#include <syscall.h>
#include <uconst.h>
#include <server/uiserver.h>
#include <ui/logstrip.h>

timedisplay *timedisplay_new(int line, int col) {
	timedisplay *time = malloc(sizeof(timedisplay));
	time->logstrip = logstrip_new(line, col, 20);
	return time;
}

void timedisplay_update(timedisplay* time, int ticks) {
	int ms = TICK2MS(ticks);
	logstrip_printf(time->logstrip, "Uptime: %d.%ds", ms / 1000, (ms / 100) % 10)
}
