#pragma once

#include <location.h>
#include <stdio.h>

#define MSPERTICK 1
#define MS2TICK(x) ((x) / MSPERTICK)
#define TICK2MS(x) ((x) * MSPERTICK)

typedef enum {
	MSG_SUB = 13,
	MSG_REQ = 17,
	MSG_SENSOR = 19,
	MSG_COM_IN = 23,
	MSG_TIME = 29,
	MSG_TRAINCMD = 31,
	MSG_TRAINCMDRECEIPT = 37,
	MSG_TRAINVCMD = 41,
	MSG_UI = 43
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
	SPEED = 47,
	REVERSE = 53,
	SWITCH = 59,
	SOLENOID = 61,
	QUERY1 = 67,
	QUERY = 71,
	GO = 73,
	STOP = 79,
	REVERSE_UI = 83
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
	VCMD_SETSPEED,
	VCMD_SETREVERSE,
	VCMD_SETSWITCH,
	VCMD_WAITFORMS,
	VCMD_WAITFORLOC,
	VCMD_STOP
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

typedef enum uimsgtype {
	UIMSG_REGISTER,	UIMSG_EFFECT, UIMSG_OUT, UIMSG_MOVE, UIMSG_FORCE_REFRESH, UIMSG_MOVECURSOR
} uimsgtype;

typedef struct {
	msgtype type;
	uimsgtype uimsg;
	int id;
	char flag;
	char fgcolor;
	int line;
	int col;
	int strlen;
	char str[];
} msg_ui;

static inline int msg_tostring(void* msg, char buf[]) {
	char *b = buf;
	msg_header *header = msg;
	switch (header->type) {
		case MSG_SUB: {
			msg_sub *m = msg;
			b += sprintf(b, "(SUB TASK %d to PUB)", m->tid);
			break;
		}
		case MSG_REQ: {
			b += sprintf(b, "(BUFFER REQ)");
			break;
		}
		case MSG_SENSOR: {
			msg_sensor *m = msg;
			b += sprintf(b, "(SENSOR %c%d %s AT UPTIME %d)", m->module[0], m->id, m->state ? "ON" : "OFF", m->timestamp);
			break;
		}
		case MSG_COM_IN: {
			msg_comin *m = msg;
			b += sprintf(b, "(CHAR '%c' on %s)", m->c, m->channel == 0 ? "COM1" : m->channel == 1 ? "COM2" : "BADCOM");
			break;
		}
		case MSG_TIME: {
			msg_time *m = msg;
			b += sprintf(b, "(TIME PING AT UPTIME %d)", m->timestamp);
			break;
		}
		case MSG_TRAINCMD: {
			traincmd *cmd = msg;
			switch (cmd->name) {
				case SPEED:
					b += sprintf(b, "(TRAIN %d SPEED %d)", cmd->arg1, cmd->arg2);
					break;
				case REVERSE:
					b += sprintf(b, "(REVERSE TRAIN %d)", cmd->arg1);
					break;
				case REVERSE_UI:
					b += sprintf(b, "(PAUSE+REVERSE TRAIN %d)", cmd->arg1);
					break;
				case SWITCH:
					b += sprintf(b, "(SWITCH BR%d to %c)", cmd->arg1, cmd->arg2);
					break;
				case QUERY1:
					b += sprintf(b, "(QUERY SENSOR MOD %d)", cmd->arg1);
					break;
				case QUERY:
					b += sprintf(b, "(QUERY THE FIRST %d SENSOR MODS)", cmd->arg1);
					break;
				case GO:
					b += sprintf(b, "(GO)");
					break;
				case STOP:
					b += sprintf(b, "(STOP)");
					break;
				default:
					b += sprintf(b, "(BAD TRAINCMD: %d)", cmd->name);
					break;
			}
			break;
		}
		case MSG_TRAINCMDRECEIPT: {
			traincmd_receipt *rcpt = msg;
			traincmd *cmd = &rcpt->cmd;
			b += sprintf(b, "(TRAIN CMD RECEIPT FOR");
			b += msg_tostring(cmd, b);
			b += sprintf(b, " AT UPTIME %d", rcpt->timestamp);
			break;
		}
		case MSG_TRAINVCMD: { // @TODO: fill in
			b += sprintf(b, "(TRAIN VCMD)");
			break;
		}
		case MSG_UI: {
			b += sprintf(b, "(UI MSG)");
			break;
		}
		default:
			b += sprintf(b, "(BAD MSG %d)", header->type);
			break;
	}
	return b - buf;
}
