#include <a0.h>
#include <train.h>
#include <syscall.h>
#include <sensornotifier.h>
#include <uconst.h>
#include <comnotifier.h>
#include <string.h>
#include <traincmdbuffer.h>
#include <stdio.h>

#define LEN_MSG (4 * 64)
#define LEN_CMD 128

typedef struct {
	// server ids
	int tid_time;
	int tid_com1;
	int tid_com2;
	int tid_traincmdbuf;
	// cmd buffer
	char cmd[LEN_CMD];
	int cmd_i;
} a0state;

void handle_sensor(a0state *state, char msg[]) {
	msg_sensor *sensor = (msg_sensor*) msg;
	char buf[100];
	sprintf(buf, "%c: %d\n", sensor->module, sensor->id);
	Putstr(COM2, buf, state->tid_com2);
}

void handle_com2in(a0state *state, msg_comin *comin) {
	ASSERT(state->cmd_i < LEN_CMD - 1, "cmd buffer full");

	switch (comin->c) {
		case '\b':
			if (state->cmd_i != 0) {
				state->cmd_i -= 1;
			}
			break;
		case '\r': {
			state->cmd[state->cmd_i] = '\n';
			char *c = state->cmd;
			#define ACCEPT(a) { if (*c++ != a) break; }

			state->cmd[state->cmd_i + 1] = '\0';
			char com2outbuf[LEN_CMD + 100], *p = com2outbuf;
			sprintf(com2outbuf, "cmd: %s", state->cmd);
			Putstr(COM2, com2outbuf, state->tid_com2);
			switch (*c++) {
				case 't': { // set train speed (tr # #)
					ACCEPT('r');
					ACCEPT(' ');
					int train = strgetui(&c);
					if (!train_goodtrain(train)) break;
					ACCEPT(' ');
					int speed = strgetui(&c);
					if (!train_goodspeed(speed)) break;
					ACCEPT('\n');
					train_speed(train, speed, state->tid_traincmdbuf);
					break;
				}
				case 'r': { // reverse train (rv #)
					ACCEPT('v');
					ACCEPT(' ');
					int train = strgetui(&c);
					if (!train_goodtrain(train)) break;
					ACCEPT('\n');
					// { ... } save train speed
					train_speed(train, 0, state->tid_traincmdbuf);
					train_reverse(train, state->tid_traincmdbuf);
					train_speed(train, 30, state->tid_traincmdbuf); // restore train speed
					break;
				}
				case 's': { // set switch position (sw # [CS])
					ACCEPT('w');
					ACCEPT(' ');
					int switchno = strgetui(&c);
					if (!train_goodswitch(switchno)) break;
					ACCEPT(' ');
					char pos = *c++;
					if (!train_goodswitchpos(pos)) break;
					ACCEPT('\n');
					train_switch(switchno, pos, state->tid_traincmdbuf);
					train_solenoidoff(state->tid_traincmdbuf);
					break;
				}
				case 'q':
					ACCEPT('\n');
					ExitKernel(0);
					break;
			}
			state->cmd_i = 0; // empty the buffer
			break;
		}
		default:
			state->cmd[state->cmd_i++] = comin->c;
			break;
	}
	state->cmd[state->cmd_i] = '\0';
	// ioprintf(state->tid_com2, "%s\n", state->cmd);
}

static void handle_comin(a0state *state, char msg[]) {
	msg_comin *comin = (msg_comin*) msg;

	switch (comin->channel) {
		case COM1:
			ERROR("com1 handler not defined");
			break;
		case COM2:
			handle_com2in(state, comin);
			break;
		default:
			ERROR("invalid channel# %d" , comin->channel);
			break;
	}
}

void a0() {
	a0state state;
	state.tid_time = WhoIs(NAME_TIMESERVER);
	state.tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	state.tid_com2 = WhoIs(NAME_IOSERVER_COM2);
	state.tid_traincmdbuf = traincmdbuffer_new();
	state.cmd[0] = '\0';
	state.cmd_i = 0;

	train_go(state.tid_traincmdbuf);

	// ioprintf(tid_com2, "switching to curved\n");
	// train_switchall('C', state.tid_traincmdbuf);
	// Delay(100, tid_time);

	train_speed(24, 14 + 16, state.tid_traincmdbuf);
	train_speed(21, 14 + 16, state.tid_traincmdbuf);
	/*Delay(500, tid_time);
	train_speed(train, 0, state.tid_traincmdbuf);
	train_reverse(train, state.tid_traincmdbuf);
	train_speed(train, speed, state.tid_traincmdbuf);*/

	sensornotifier_new();
	comnotifier_new(10, MyTid(), COM2, state.tid_com2);

	for (;;) {
		int tid;
		char msg[LEN_MSG];
		int rcvlen = Receive(&tid, msg, LEN_MSG);
		ASSERT(rcvlen >= sizeof(msg_header), "bad data");
		ReplyNull(tid);
		msg_header *header = (msg_header*) msg;
		switch (header->type) {
			case SENSOR:
				handle_sensor(&state, msg);
				break;
			case COM_IN:
				handle_comin(&state, msg);
				break;
			default:
				break;
		}
	}
}
