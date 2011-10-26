#include <a0.h>
#include <train.h>
#include <syscall.h>
#include <sensornotifier.h>
#include <uconst.h>
#include <comnotifier.h>
#include <string.h>
#include <traincmdbuffer.h>
#include <stdio.h>
#include <timenotifier.h>

#define LEN_MSG (64 * 4)
#define LEN_CMD 32

typedef struct {
	// server ids
	int tid_time;
	int tid_com1;
	int tid_com2;
	int tid_traincmdbuf;
	// cmd buffer
	char cmd[LEN_CMD];
	int cmd_i;
	// train data
	char train_speed[TRAIN_MAX_TRAIN_ADDR + 1];
	// ui
} a0state;

/*
 * UI
 */

static inline void ui_time(a0state *state, int ticks) {
	char buf[100];
	sprintf(buf, "ticks: %d\n", ticks);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_init(a0state *state) {
	Putstr(COM2, "\x1B[2J", state->tid_com2); // clear screen
}

static inline void ui_sensor(a0state *state, char module, int id) {
	char buf[100];
	sprintf(buf, "%c: %d\n", module, id);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_speed(a0state *state, int train, int speed) {
	char buf[100];
	sprintf(buf, "setting speed of train %d to %d\n", train, speed);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_reverse(a0state *state, int train) {
	char buf[100];
	sprintf(buf, "reversing train %d\n", train);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_switch(a0state *state, char no, char pos) {
	char buf[100];
	sprintf(buf, "switching switch %d to '%c'\n", no, pos);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_switchall(a0state *state, char pos) {
	char buf[100];
	sprintf(buf, "switching all switches to '%c'\n", pos);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_cmd_char(a0state *state, char c) {
	Putc(COM2, c, state->tid_com2);
}

static inline void ui_cmd_delchar(a0state *state) {
	// delete rightmost command character
}

static inline void ui_cmd_clear(a0state *state) {
	// delete all command characters
	Putc(COM2, '\n', state->tid_com2);
	Putc(COM2, '\n', state->tid_com2);
}

static inline void ui_cmd(a0state *state, char cmd[]) {
	Putstr(COM2, cmd, state->tid_com2);
	Putc(COM2, '\n', state->tid_com2);
}

static inline void ui_cmd_bad(a0state *state) {
	Putstr(COM2, "bad command\n", state->tid_com2);
}

static inline void ui_quit(a0state *state) {
	Putstr(COM2, "bye bye\n", state->tid_com2);
}

/*
 * Server
 */

static inline void handle_sensor(a0state *state, char msg[]) {
	msg_sensor *sensor = (msg_sensor*) msg;
	ui_sensor(state, sensor->module, sensor->id);
}

#define ACCEPT(a) { \
	if (*c++ != a) { \
		goodcmd = FALSE; \
		ui_cmd_bad(state); \
		break; \
	} \
}

static inline void handle_com2in(a0state *state, msg_comin *comin) {
	if (state->cmd_i + 1 == LEN_CMD) return; // full, ignore
	state->cmd[state->cmd_i++] = comin->c;
	state->cmd[state->cmd_i] = '\0';
	switch (comin->c) {
		case '\b':
			if (state->cmd_i >= 2) {
				state->cmd_i -= 2;
			}
			ui_cmd_delchar(state);
			break;
		case '\r': {
			int goodcmd = TRUE;
			char *c = state->cmd;
			switch (*c++) {
				case 't': { // set train speed (tr # #)
					ACCEPT('r');
					ACCEPT(' ');
					int train = strgetui(&c);
					if (!train_goodtrain(train)) break;
					ACCEPT(' ');
					int speed = strgetui(&c);
					if (!train_goodspeed(speed)) break;
					ACCEPT('\r');
					ui_speed(state, train, speed);
					state->train_speed[train] = speed;
					train_speed(train, speed, state->tid_traincmdbuf);
					break;
				}
				case 'r': { // reverse train (rv #)
					ACCEPT('v');
					ACCEPT(' ');
					int train = strgetui(&c);
					if (!train_goodtrain(train)) break;
					ACCEPT('\r');
					int speed = state->train_speed[train];
					ui_reverse(state, train);
					train_speed(train, 0, state->tid_traincmdbuf);
					train_reverse(train, state->tid_traincmdbuf);
					train_speed(train, speed, state->tid_traincmdbuf);
					break;
				}
				case 's': { // set switch position (sw [#*] [cCsS])
					ACCEPT('w');
					ACCEPT(' ');
					char switchno;
					if (*c == '*') {
						switchno = *c++;
					} else {
						switchno = strgetui(&c);
						if (!train_goodswitch(switchno)) break;
					}
					ACCEPT(' ');
					char pos = *c++;
					if (!train_goodswitchpos(pos)) break;
					ACCEPT('\r');
					if (switchno == '*') {
						ui_switchall(state, pos);
						train_switchall(pos, state->tid_traincmdbuf);
					} else {
						ui_switch(state, switchno, pos);
						train_switch(switchno, pos, state->tid_traincmdbuf);
					}
					train_solenoidoff(state->tid_traincmdbuf);
					break;
				}
				case 'q': { // quit kernel
					ACCEPT('\r');
					ui_quit(state);
					train_stop(state->tid_traincmdbuf);
					Flush(state->tid_com1);
					Flush(state->tid_com2);
					ExitKernel(0);
					break;
				}
			}
			if (goodcmd) {
				ui_cmd_clear(state);
				ui_cmd(state, state->cmd);
			}
			state->cmd[0] = '\0';
			state->cmd_i = 0;
			break;
		}
		default:
			ui_cmd_char(state, comin->c);
			break;
	}
}

static inline void handle_comin(a0state *state, char msg[]) {
	msg_comin *comin = (msg_comin*) msg;
	ASSERT(comin->channel == COM2, "no handler for channel %d", comin->channel);
	handle_com2in(state, comin);
}

static inline void handle_time(a0state *state, char msg[]) {
	msg_time *time = (msg_time*) msg;
	ui_time(state, time->ticks);
}

void a0() {
	a0state state;
	state.tid_time = WhoIs(NAME_TIMESERVER);
	state.tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	state.tid_com2 = WhoIs(NAME_IOSERVER_COM2);
	state.tid_traincmdbuf = traincmdbuffer_new();
	state.cmd[0] = '\0';
	state.cmd_i = 0;
	for (int i = 0; i <= TRAIN_MAX_TRAIN_ADDR; i++) {
		state.train_speed[i] = 0;
	}

	ui_init(&state);

	train_go(state.tid_traincmdbuf);

	sensornotifier_new(MyTid());
	comnotifier_new(MyTid(), 10, COM2, state.tid_com2);
	timenotifier_new(MyTid(), 10, 100);

	for (;;) {
		int tid;
		char msg[LEN_MSG];
		int rcvlen = Receive(&tid, msg, LEN_MSG);
		ASSERT(rcvlen >= sizeof(msg_header), "bad data");
		Reply(tid, NULL, 0);
		msg_header *header = (msg_header*) msg;
		switch (header->type) {
			case SENSOR:
				handle_sensor(&state, msg);
				break;
			case COM_IN:
				handle_comin(&state, msg);
				break;
			case TIME:
				handle_time(&state, msg);
				break;
			default:
				break;
		}
	}
}
