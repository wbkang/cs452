#include <ui/logstrip.h>
#include <syscall.h>

logstrip *logstrip_new(console *con, int line, int col) {
	logstrip *this = malloc(sizeof(logstrip));
	this->con = con;
	this->line = line;
	this->col = col;
	return this;
}
