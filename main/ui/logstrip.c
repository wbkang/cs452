#include <ui/logstrip.h>

logstrip logstrip_create(int line, int col, console *con) {
	logstrip l;
	l.con= con;
	l.line = line;
	l.col = col;
	return l;
}
