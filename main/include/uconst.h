#pragma once

#define MSPERTICK 10

typedef enum {SENSOR, COM_IN, TIME} msgtype;

typedef struct {
	msgtype type;
} msg_header;

typedef struct {
	msgtype type;
	char module;
	int id;
} msg_sensor;

typedef struct {
	msgtype type;
	char channel;
	char c;
} msg_comin;

typedef struct {
	msgtype type;
	int ticks;
} msg_time;
