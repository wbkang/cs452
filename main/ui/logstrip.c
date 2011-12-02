#include <ui/logstrip.h>
#include <syscall.h>
#include <server/uiserver.h>

#define DEFAULT_LOGSTRIP_WIDTH 60

logstrip *logstrip_new(int line, int col, int width) {
	logstrip *this = malloc(sizeof(logstrip));
	int tid_ui = WhoIs(NAME_UISERVER);
	this->tid_ui = tid_ui;
	this->id_ui = uiserver_register(tid_ui);
	if (width == -1) {
		width = DEFAULT_LOGSTRIP_WIDTH;
	}
	this->width = width;
	this->line = line;
	this->col = col;
	return this;
}
