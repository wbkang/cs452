#pragma once

#define MSPERTICK 1
#define MS2TICK(x) ((x) / MSPERTICK)
#define TICK2MS(x) ((x) * MSPERTICK)

typedef enum {
	SENSOR, COM_IN, TIME, REQ, TRAINCMD, TRAINCMDRECEIPT, SUB
} msgtype;

typedef struct {
	msgtype type;
} msg_header;

typedef struct {
	msgtype type;
	int timestamp;
	char module[2];
	int id;
	int state;
} msg_sensor;

typedef struct {
	msgtype type;
	char channel;
	char c;
} msg_comin;

typedef struct {
	msgtype type;
	int timestamp;
} msg_time;

typedef enum {
	SPEED, REVERSE, SWITCH, SOLENOID, QUERY1, QUERY, GO, STOP, PAUSE
} traincmdname;

typedef struct {
	msgtype type;
	traincmdname name;
	int arg1;
	int arg2;
} traincmd;

typedef struct {
	msgtype type;
	int timestamp;
	traincmd cmd;
} traincmd_receipt;

typedef struct {
	msgtype type;
	int tid;
} msg_sub;
