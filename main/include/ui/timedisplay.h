#pragma once
#include <console.h>

typedef struct timedisplay timedisplay;

struct timedisplay {
	console *con;
	int line, col;
};

timedisplay *timedisplay_new(console *con, int line, int col);
void timedisplay_update(timedisplay* time, int ticks);
