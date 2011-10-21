#pragma once

typedef enum {SENSOR, COM1OUT, COM2OUT} msgtype;

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
	char c;
} msg_com2out;
