#include <a0.h>
#include <train.h>
#include <syscall.h>
#include <sensornotifier.h>
#include <uconst.h>
#include <comnotifier.h>
#include <string.h>
#include <traincmdbuffer.h>
#include <traincmdrunner.h>
#include <stdio.h>
#include <timenotifier.h>
#include <console.h>
#include <a0_track_template.h>

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
} a0state;

/*
 * UI
 */

#define LEN_SENSOR_HIST 8
static char hist_mod[LEN_SENSOR_HIST];
static int hist_id[LEN_SENSOR_HIST];

typedef struct {
	int row, col;
	char straight, curved;
} switch_pic_info;

static switch_pic_info switch_pic_info_table[22] = {
		{ 23, 7, '\\', '=' }, 	// 1
		{ 25, 9, '\\', '=' },	// 2
		{ 27, 11, '=', '\\' },	// 3
		{ 11, 7, '/', '=' },	// 4
		{ 27, 39, '=', '/' },	// 5
		{ 25, 29, '=', '\\' },	// 6
		{ 25, 40, '=', '/' },	// 7
		{ 23, 52, '/', '=' },	// 8
		{ 11, 52, '\\', '=' },	// 9
		{ 11, 38, '=', '/' },	// 10
		{ 9, 16, '=', '/' },	// 11
		{ 9, 9, '=', '/' },		// 12
		{ 11, 28, '=', '\\' },	// 13
		{ 11, 15, '/', '=' },	// 14
		{ 23, 15, '\\', '=' },	// 15
		{ 23, 29, '=', '/' },	// 16
		{ 23, 39, '=', '\\' },	// 17
		{ 27, 30, '=', '\\' },	// 18
		{ 19, 33, '|', '/' },	// 153
		{ 18, 34, '|', '\\' },	// 154
		{ 15, 34, '|', '/' },	// 155
		{ 16, 33, '|', '\\' }	// 156
};

typedef struct {
	enum direction { UNKNOWN, NORTH, SOUTH, EAST, WEST, NORTHEAST, SOUTHEAST, NORTHWEST, SOUTHWEST } dir;
	int row, col;
} sensor_pic_info;

static char direction_str[9][3] = { "", "/\\", "\\/", ">", "<", "/\\", "\\/", "/\\", "\\/" };

static sensor_pic_info sensor_pic_info_table[TRAIN_NUM_MODULES][TRAIN_NUM_SENSORS];

static inline void sensor_pic_def(
		char mod,
		int id1, enum direction dir1,
		int id2, enum direction dir2,
		int row, int col) {
	sensor_pic_info_table[mod - 'A'][id1].dir = dir1;
	sensor_pic_info_table[mod - 'A'][id1].row = row;
	sensor_pic_info_table[mod - 'A'][id1].col = col;
	sensor_pic_info_table[mod - 'A'][id2].dir = dir2;
	sensor_pic_info_table[mod - 'A'][id2].row = row;
	sensor_pic_info_table[mod - 'A'][id2].col = col;
}

static inline void ui_cmd_clear(a0state *state);

static inline void ui_init(a0state *state) {
	char buf[128];
	char *b = buf;
	b += console_clear(b);
	b += console_cursor_move(b, 1, 1);
	Putstr(COM2, buf, state->tid_com2);

	// init sensor hist
	for (int i = 0; i < LEN_SENSOR_HIST; i++) {
		hist_mod[i] = 0;
		hist_id[i] = 0;
	}

	for (int i = 0; i < TRAIN_NUM_MODULES; i++) {
		for (int j = 0; j < TRAIN_NUM_SENSORS; j++) {
			sensor_pic_info_table[i][j].dir = UNKNOWN;
		}
	}

	sensor_pic_def('A', 1, EAST, 2, WEST, 9, 6);
	sensor_pic_def('C', 13, EAST, 14, WEST, 9 ,19);
	sensor_pic_def('E', 7, EAST, 8, WEST, 9, 33);
	sensor_pic_def('D', 7, EAST, 8, WEST, 9, 48);
	sensor_pic_def('A', 13, EAST, 14, WEST, 11, 4);
	sensor_pic_def('C', 11, EAST, 12, WEST, 11, 18);
	sensor_pic_def('B', 5, EAST, 6, WEST, 11, 31);
	sensor_pic_def('D', 3, EAST, 4, WEST, 11, 35);
	sensor_pic_def('E', 5, EAST, 6, WEST, 11, 41);
	sensor_pic_def('D', 6, EAST, 5, WEST, 11, 49);
	sensor_pic_def('E', 15, NORTHWEST, 16, SOUTHEAST, 12, 28);
	sensor_pic_def('E', 4, NORTHEAST, 3, SOUTHWEST, 12, 37);
	sensor_pic_def('A', 16, EAST, 15, WEST, 13, 3);
	sensor_pic_def('E', 2, NORTHWEST, 1, SOUTHEAST, 13, 29);
	sensor_pic_def('D', 2, NORTHEAST, 1, SOUTHWEST, 13, 36);
	sensor_pic_def('A', 3, NORTH, 4, SOUTH, 14, 14);
	sensor_pic_def('B', 15, NORTH, 16, SOUTH, 20, 14);
	sensor_pic_def('A', 11, EAST, 12, WEST, 21, 2);
	sensor_pic_def('C', 2, NORTHEAST, 1, SOUTHWEST, 21, 30);
	sensor_pic_def('B', 13, NORTHWEST, 14, SOUTHEAST, 21, 37);
	sensor_pic_def('B', 3, NORTHEAST, 4, SOUTHWEST, 22, 29);
	sensor_pic_def('D', 15, NORTHWEST, 16, SOUTHEAST, 22, 38);
	sensor_pic_def('B', 7, EAST, 8, WEST, 23, 3);
	sensor_pic_def('A', 10, EAST, 9, WEST, 23, 5);
	sensor_pic_def('C', 10, EAST, 9, WEST, 23, 18);
	sensor_pic_def('B', 1, EAST, 2, WEST, 23, 32);
	sensor_pic_def('D', 14, EAST, 13, WEST, 23, 36);
	sensor_pic_def('E', 14, EAST, 13, WEST, 23, 42);
	sensor_pic_def('E', 9, EAST, 10, WEST, 23, 49);
	sensor_pic_def('B', 11, EAST, 12, WEST, 25, 3);
	sensor_pic_def('A', 8, EAST, 7, WEST, 25, 6);
	sensor_pic_def('C', 5, EAST, 6, WEST, 25, 19);
	sensor_pic_def('C', 15, EAST, 16, WEST, 25, 32);
	sensor_pic_def('D', 12, EAST, 11, WEST, 25, 37);
	sensor_pic_def('E', 11, EAST, 12, WEST, 25, 43);
	sensor_pic_def('D', 10, EAST, 9, WEST, 25, 48);
	sensor_pic_def('B', 9, EAST, 10, WEST, 27, 3);
	sensor_pic_def('A', 5, EAST, 6, WEST, 27, 8);
	sensor_pic_def('C', 7, EAST, 8, WEST, 27, 27);
	sensor_pic_def('C', 3, EAST, 4, WEST, 27, 42);

	Putstr(COM2, TRACK_TEMPLATE, state->tid_com2);

	ui_cmd_clear(state);

	return;
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

static inline int ui_move2log(char * str) {
	char * const orig_str = str;
	str += console_cursor_move(str, 29, 1);
	str += console_erase_eol(str);
	return str - orig_str;
}

static inline void ui_sensor(a0state *state, char module, int id) {
	for (int i = LEN_SENSOR_HIST - 1; i > 0; i--) {
		hist_mod[i] = hist_mod[i - 1];
		hist_id[i] = hist_id[i - 1];
	}
	hist_mod[0] = module;
	hist_id[0] = id;

	char buf[1024];
	char *b = buf;
	b += console_cursor_save(b);
	b += console_cursor_move(b, 6, 17);
	b += console_erase_eol(b);

	int max_hist_idx = 0;

	for (int i = 0; (i < LEN_SENSOR_HIST) && hist_mod[i]; i++) {
		b += sprintf(b, "%c%d, ", hist_mod[i], hist_id[i]);
		if (i == LEN_SENSOR_HIST) {
			b += sprintf(b, "...");
		}

		max_hist_idx = i;
	}

	for (int i = max_hist_idx; i >= 0; i--) {
		sensor_pic_info spinfo = sensor_pic_info_table[hist_mod[i]-'A'][hist_id[i]];

		if (spinfo.dir != UNKNOWN) {
			b += console_cursor_move(b, spinfo.row, spinfo.col);

			switch (i) {
				case 0:
					b += sprintf(b, CONSOLE_EFFECT(EFFECT_BRIGHT));
				case 1:
					b += sprintf(b, CONSOLE_EFFECT(EFFECT_FG_CYAN));
					break;
				default:
					b += sprintf(b, CONSOLE_EFFECT(EFFECT_FG_BLUE));
					break;
			}

			b += sprintf(b, "%s" CONSOLE_EFFECT(EFFECT_RESET), direction_str[spinfo.dir]);
		}
	}

	b += console_cursor_unsave(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_speed(a0state *state, int train, int speed) {
	char buf[128];
	char *b = buf;
	b += console_cursor_save(b);
	b += ui_move2log(b);
	b += sprintf(b, "set speed of train %d to %d", train, speed);
	b += console_cursor_unsave(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_reverse(a0state *state, int train) {
	char buf[128];
	char *b = buf;
	b += console_cursor_save(b);
	b += ui_move2log(b);
	b += sprintf(b, "reversed train %d", train);
	b += console_cursor_unsave(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline int ui_updateswitchstatus(char *buf, char no, char pos) {
	char * const orig_buf = buf;
	int idx = train_switchno2i(no); // 0 based
	int statusrow = 2 + idx / 6;
	int statuscol = 14 + 5 * (idx % 6);
	char pos_name = train_switchpos_straight(pos) ? 'S' : 'C';

	buf += console_cursor_move(buf, statusrow, statuscol);
	buf += sprintf(buf, CONSOLE_EFFECT(EFFECT_BRIGHT) CONSOLE_EFFECT(EFFECT_FG_YELLOW)
			"%c" CONSOLE_EFFECT(EFFECT_RESET), pos_name);

	switch_pic_info swinfo = switch_pic_info_table[idx];

	buf += console_cursor_move(buf, swinfo.row, swinfo.col);
	buf += sprintf(buf,
			CONSOLE_EFFECT(EFFECT_BRIGHT)
			CONSOLE_EFFECT(EFFECT_FG_YELLOW)
			"%c"
			CONSOLE_EFFECT(EFFECT_RESET),
			(pos_name == 'S') ? swinfo.straight : swinfo.curved);

	return buf - orig_buf;
}

static inline void ui_switch(a0state *state, char no, char pos) {
	char buf[1024];
	char *b = buf;
	b += console_cursor_save(b);
	b += ui_move2log(b);
	b += sprintf(b, "switched switch %d to %c", no, pos);
	b += ui_updateswitchstatus(buf, no, pos);
	b += console_cursor_unsave(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_switchall(a0state *state, char pos) {
	char buf[4096];
	char *b = buf;
	b += console_cursor_save(b);
	b += ui_move2log(b);
	b += sprintf(b, "switched all switches to '%c'", pos);
	for (int i = 0; i < TRAIN_NUM_SWITCHADDR; i++) {
		b += ui_updateswitchstatus(b, train_switchi2no(i), pos);
	}
	b += console_cursor_unsave(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_setup_demo_track(a0state *state) {
	char buf[1024];
	char *b = buf;
	ui_switchall(state, 'C');
	b += console_cursor_save(b);
	b += ui_updateswitchstatus(b, 9, 'S');
	b += ui_updateswitchstatus(b, 10, 'S');
	b += ui_updateswitchstatus(b, 15, 'S');
	b += ui_updateswitchstatus(b, 16, 'S');
	b += ui_updateswitchstatus(b, 14, 'S');
	b += ui_move2log(b);
	b += sprintf(b, "adjusted all switches for demo");
	b += console_cursor_unsave(b);

	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_cmd_char(a0state *state, char c, int cmdpos) {
	char buf[128];
	char *b = buf;
	b += console_cursor_move(b, 30, 2 + cmdpos);
	b += sprintf(b, CONSOLE_EFFECT(EFFECT_RESET) "%c", c);
	Putstr(COM2, buf, state->tid_com2);
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
	b += console_cursor_move(b, 30, 3);
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
	b += ui_move2log(b);
	b += sprintf(b, "invalid command: \"%s\"", cmd);
	b += console_cursor_unsave(b);
	Putstr(COM2, buf, state->tid_com2);
}

static inline void ui_quit(a0state *state) {
	char buf[128];
	char *b = buf;
	b += console_cursor_save(b);
	b += ui_move2log(b);
	b += sprintf(b, "quitting...");
	b += console_cursor_move(b, 31, 1);
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
				case 'd': {
					train_switchall('C', state->tid_traincmdbuf);
					train_switch(9, 'S', state->tid_traincmdbuf);
					train_switch(10, 'S', state->tid_traincmdbuf);
					train_switch(15, 'S', state->tid_traincmdbuf);
					train_switch(16, 'S', state->tid_traincmdbuf);
					train_switch(14, 'S', state->tid_traincmdbuf);
					ui_setup_demo_track(state);
					break;
				}
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
			ui_cmd_char(state, comin->c, state->cmd_i);
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
	traincmdrunner_new();
	state.cmd[0] = '\0';
	state.cmd_i = 0;
	for (int i = 0; i <= TRAIN_MAX_TRAIN_ADDR; i++) {
		state.train_speed[i] = 0;
	}

	ui_init(&state);

	train_go(state.tid_traincmdbuf);

	sensornotifier_new(MyTid());
	comnotifier_new(MyTid(), 10, COM2, state.tid_com2);
	timenotifier_new(MyTid(), 10, MS2TICK(100));

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
