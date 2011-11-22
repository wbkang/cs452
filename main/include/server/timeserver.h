#pragma once

#include <constants.h>
#include <util.h>

#define TIMESERVER_TICK 0
#define TIMESERVER_TIME 1
#define TIMESERVER_DELAY 2
#define TIMESERVER_DELAYUNTIL 3

#define TIMESERVER_ERROR_BADREQNO -1
#define TIMESERVER_ERROR_BADDATA -2

void timeserver();

int timeserver_create();
