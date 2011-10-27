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
#include <console.h>

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

#define LEN_SENSOR_HIST 8
static char hist_mod[LEN_SENSOR_HIST];
static int hist_id[LEN_SENSOR_HIST];

static inline void ui_init(a0state *state) {
	char buf[128];
	char *b = buf;
	b += console_clear(b);
	b += console_cursor_move(b, 1, 1);
	b += sprintf(b, "uptime:");
	b += console_cursor_move(b, 2, 1);
	b += sprintf(b, "switches:");
	for (int i = 0; i < TRAIN_NUM_SWITCHADDR; i++) {
		int switchno = train_switchi2no(i);
		if (switchno < 100) b += sprintf(b, " ");
		if (switchno < 10) b += sprintf(b, " ");
		b += sprintf(b, " %d?", switchno);
	}
	b += console_cursor_move(b, 3, 1);
	b += sprintf(b, "recent sensor:");
	b += console_cursor_move(b, 4, 1);
	b += sprintf(b, "last cmd:");
	b += console_cursor_move(b, 5, 1);
	b += sprintf(b, "$");
	Putstr(COM2, buf, state->tid_com2);
	// init sensor hist
	for (int i = LEN_SENSOR_HIST - 1; i > 0; i--) {
		hist_mod[i] = 0;
		hist_id[i] = 0;
	}
}

static inline void ui_time(a0state *state, int ticks) {
	char buf[32];
	char *b = buf;
	b += console_cursor_save(b);
	b += console_cursor_move(b, 1, 9);
	b += console_erase_eol(b);
	ticks /= 10; // time in 100ms
	b += sprintf(b, "%d.%ds", ticks / 10, ticks % 10);
	b += console_cursor_unsave(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_sensor(a0state *state, char module, int id) {
	for (int i = LEN_SENSOR_HIST - 1; i > 0; i--) {
		hist_mod[i] = hist_mod[i - 1];
		hist_id[i] = hist_id[i - 1];
	}
	hist_mod[0] = module;
	hist_id[0] = id;

	char buf[256];
	char *b = buf;
	b += console_cursor_save(b);
	b += console_cursor_move(b, 3, 16);
	b += console_erase_eol(b);
	int i;
	for (i = 0; (i < LEN_SENSOR_HIST) && hist_mod[i]; i++) {
		b += sprintf(b, "%c%d, ", hist_mod[i], hist_id[i]);
	}
	if (i == LEN_SENSOR_HIST) {
		b += sprintf(b, "...");
	}
	b += console_cursor_unsave(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_speed(a0state *state, int train, int speed) {
	char buf[128];
	char *b = buf;
	b += console_cursor_save(b);
	b += console_cursor_move(b, 4, 11);
	b += console_erase_eol(b);
	b += sprintf(b, "set speed of train %d to %d", train, speed);
	b += console_cursor_unsave(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_reverse(a0state *state, int train) {
	char buf[128];
	char *b = buf;
	b += console_cursor_save(b);
	b += console_cursor_move(b, 4, 11);
	b += console_erase_eol(b);
	b += sprintf(b, "reversed train %d", train);
	b += console_cursor_unsave(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_switch(a0state *state, char no, char pos) {
	char buf[128];
	char *b = buf;
	b += console_cursor_save(b);
	b += console_cursor_move(b, 4, 11);
	b += console_erase_eol(b);
	b += sprintf(b, "switched switch %d to %c", no, pos);
	b += console_cursor_move(b, 2, 9 + 5 * train_switchno2i(no) + 5);
	b += sprintf(b, "%c", train_switchpos_straight(pos) ? 'S' : 'C');
	b += console_cursor_unsave(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_switchall(a0state *state, char pos) {
	char buf[128];
	char *b = buf;
	b += console_cursor_save(b);
	b += console_cursor_move(b, 4, 11);
	b += console_erase_eol(b);
	b += sprintf(b, "switched all switches to '%c'", pos);
	for (int i = 0; i < TRAIN_NUM_SWITCHADDR; i++) {
		b += console_cursor_move(b, 2, 9 + 5 * i + 5);
		b += sprintf(b, "%c", train_switchpos_straight(pos) ? 'S' : 'C');
	}
	b += console_cursor_unsave(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_cmd_char(a0state *state, char c) {
	Putc(COM2, c, state->tid_com2);
}

static inline void ui_cmd_delchar(a0state *state) {
	char buf[128];
	char *b = buf;
	b += console_cursor_left(b, 1);
	b += console_erase_eol(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_cmd_clear(a0state *state) {
	char buf[128];
	char *b = buf;
	b += console_cursor_move(b, 5, 2);
	b += console_erase_eol(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_cmd(a0state *state, char cmd[]) {
	// exact command if good
}

static inline void ui_cmd_bad(a0state *state, char cmd[]) {
	char buf[128];
	char *b = buf;
	b += console_cursor_save(b);
	b += console_cursor_move(b, 4, 11);
	b += console_erase_eol(b);
	b += sprintf(b, "invalid command: \"%s\"", cmd);
	b += console_cursor_unsave(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_quit(a0state *state) {
	char buf[128];
	char *b = buf;
	b += console_cursor_save(b);
	b += console_cursor_move(b, 4, 11);
	b += console_erase_eol(b);
	b += sprintf(b, "quitting...");
	b += console_cursor_move(b, 7, 1);
	Putstr(COM2, buf, state->tid_com2);
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
		goto badcmd; \
	} \
}

static inline void handle_com2in(a0state *state, msg_comin *comin) {
	if (state->cmd_i + 2 == LEN_CMD && comin->c != '\b' && comin->c != '\r') return; // full, ignore
	state->cmd[state->cmd_i++] = comin->c;
	state->cmd[state->cmd_i] = '\0';
	int quit = FALSE;
	switch (comin->c) {
		case '\b':
			state->cmd_i--;
			if (state->cmd_i >= 1) {
				state->cmd_i -= 1;
				ui_cmd_delchar(state);
			}
			break;
		case '\r': {
			char *c = state->cmd;
			switch (*c++) {
				case 't': { // set train speed (tr # #)
					ACCEPT('r');
					ACCEPT(' ');
					int train = strgetui(&c);
					if (!train_goodtrain(train)) goto badcmd;
					ACCEPT(' ');
					int speed = strgetui(&c);
					if (!train_goodspeed(speed)) goto badcmd;
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
					if (!train_goodtrain(train)) goto badcmd;
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
						if (!train_goodswitch(switchno)) goto badcmd;
					}
					ACCEPT(' ');
					char pos = *c++;
					if (!train_goodswitchpos(pos)) goto badcmd;
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
					quit = TRUE;
					break;
				}
				default: {
					goto badcmd;
					break;
				}
			}
			ui_cmd(state, state->cmd);
			goto skipbad;
			badcmd:
			state->cmd[state->cmd_i - 1] = '\0';
			ui_cmd_bad(state, state->cmd);
			skipbad:
			ui_cmd_clear(state);
			state->cmd[0] = '\0';
			state->cmd_i = 0;
			break;
		}
		default:
			ui_cmd_char(state, comin->c);
			break;
	}
	if (quit) {
		ui_quit(state);
		train_stop(state->tid_traincmdbuf);
		Flush(state->tid_com1);
		Flush(state->tid_com2);
		ExitKernel(0);
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
	timenotifier_new(MyTid(), 10, 10);

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
