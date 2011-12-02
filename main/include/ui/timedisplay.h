#pragma once

typedef struct timedisplay timedisplay;

struct timedisplay {
	int tid_ui;
	int id_ui;
	int line, col;
};

timedisplay *timedisplay_new(int line, int col);
void timedisplay_update(timedisplay* time, int ticks);
