#pragma once

typedef enum {SENSOR, COM_IN} msgtype;

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
