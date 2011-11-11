#pragma once

#define MSPERTICK 1
#define MS2TICK(x) ((x) / MSPERTICK)
#define TICK2MS(x) ((x) * MSPERTICK)

typedef enum {SENSOR, COM_IN, TIME} msgtype;

typedef struct {
	msgtype type;
} msg_header;

typedef struct {
	msgtype type;
	int ticks;
	char module[2];
	int id;
	int state;
} msg_sensor;

typedef struct {
	msgtype type;
	// int ticks;
	char channel;
	char c;
} msg_comin;

typedef struct {
	msgtype type;
	int ticks;
} msg_time;
