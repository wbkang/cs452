#pragma once

#include <constants.h>
#include <util.h>

#define TIMESERVER_TICK 0
#define TIMESERVER_TIME 1
#define TIMESERVER_DELAY 2
#define TIMESERVER_DELAYUNTIL 3

#define TIMESERVER_ERROR_BADREQNO -1
#define TIMESERVER_ERROR_BADDATA -2

#define TIMESERVER_RATE 10 // ms

void timeserver();
inline int timeserver_time(int timeserver);
inline int timeserver_delayuntil(int ticks, int timeserver);
inline int timeserver_delay(int ticks, int timeserver);
