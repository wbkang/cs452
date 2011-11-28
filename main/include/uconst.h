#pragma once

#include <location.h>

#define MSPERTICK 1
#define MS2TICK(x) ((x) / MSPERTICK)
#define TICK2MS(x) ((x) * MSPERTICK)

typedef enum {
	MSG_SENSOR, MSG_COM_IN, MSG_TIME, MSG_REQ, MSG_TRAINCMD, MSG_TRAINCMDRECEIPT, MSG_SUB, MSG_TRAINVCMD,
	MSG_UI
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
	SPEED, REVERSE, SWITCH, SOLENOID, QUERY1, QUERY, GO, STOP, REVERSE_UI
} traincmdname;

typedef struct {
	msgtype type;
	int timestamp; // @TODO: temp
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

typedef enum {
	VCMD_SETSPEED, VCMD_SETREVERSE, VCMD_SETSWITCH, VCMD_WAITFORMS, VCMD_WAITFORLOC, VCMD_STOP
} trainvcmdname;

typedef struct {
	msgtype type;
	trainvcmdname name;
	union {
		int speed;
		struct {
			char nodename[8];
			char pos;
		} switchinfo;
		int timeout;
	} data;
	location location;
} trainvcmd;

typedef enum {
	UIMSG_LOG
} uimsgtype;

typedef struct {
	msgtype type;
	uimsgtype uitype;
	int id;
	int strlen;
	char str[];
} msg_ui;
