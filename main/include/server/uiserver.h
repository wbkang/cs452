#pragma once

#include <constants.h>
#include <syscall.h>
#include <ui/logdisplay.h>

#define UI_A0LOG 0
#define UI_TRAINLOCLOG 1
#define UI_TRAINATTRLOG 2
#define UI_ENG_TRIPLOG 3
#define NUM_LOGS 4 // keep this in sync

void uiserver();
void uiserver_log(int logid, char *str, int tid_ui);


#define uiserver_logprintf(logid, ...) { \
	char __logbuf[MAX_LOG_COL]; \
	sprintf(__logbuf, __VA_ARGS__); \
    uiserver_log(logid, __logbuf, WhoIs(NAME_SENSORPUB)); \
}
