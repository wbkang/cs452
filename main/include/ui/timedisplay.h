#pragma once
#include <server/uiserver.h>

typedef struct timedisplay timedisplay;

struct timedisplay {
	ui_id id_ui;
	int line, col;
};

timedisplay *timedisplay_new(int line, int col);
void timedisplay_update(timedisplay* time, int ticks);
