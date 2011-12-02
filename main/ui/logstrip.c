#include <ui/logstrip.h>
#include <syscall.h>
#include <server/uiserver.h>

#define DEFAULT_LOGSTRIP_WIDTH 60

logstrip *logstrip_new(int line, int col, int width) {
	logstrip *this = malloc(sizeof(logstrip));
	this->id_ui = uiserver_register();
	this->width = width;
	this->line = line;
	this->col = col;
	return this;
}
