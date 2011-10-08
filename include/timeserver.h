#pragma once

#define TIMESERVER_NAME "ts"

#define TIMESERVER_TIME 1
#define TIMESERVER_DELAYUNTIL 2

#define TIMESERVER_ERROR_BADDATA -3
#define TIMESERVER_ERROR_BADREQNO -4

void timeserver();

int timeserver_time();

int timeserver_delay(int ticks);

int timeserver_delayuntil(int ticks);
