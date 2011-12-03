#pragma once
#include <server/uiserver.h>
#include <ui/logstrip.h>
typedef struct timedisplay timedisplay;

struct timedisplay {
	logstrip* logstrip;
};

timedisplay *timedisplay_new(int line, int col);
void timedisplay_update(timedisplay* time, int ticks);
