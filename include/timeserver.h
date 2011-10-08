#pragma once

#define TIMESERVER_NAME "ts"

#define TIMESERVER_DONTUNBLOCK 1

#define TIMESERVER_TICK 0
#define TIMESERVER_TIME 1
#define TIMESERVER_DELAYUNTIL 2

#define TIMESERVER_ERROR_BADREQNO -1
#define TIMESERVER_ERROR_BADDATA -2

void timeserver();

inline int timeserver_tick();

inline int timeserver_time();

inline int timeserver_delay(int ticks);

inline int timeserver_delayuntil(int ticks);
