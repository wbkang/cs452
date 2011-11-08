#include <train.h>
#include <syscall.h>
#include <uconst.h>
#include <string.h>
#include <stdio.h>
#include <console.h>
#include <lookup.h>
#include <track_node.h>
#include <track_data.h>
#include <fixed.h>
#include <dumbbus.h>
#include <betaimporter.h>
#include <server/sensornotifier.h>
#include <server/comnotifier.h>
#include <server/timenotifier.h>
#include <server/traincmdbuffer.h>
#include <server/traincmdrunner.h>
#include <task/a0.h>
#include <task/a0_track_template.h>
#include <ui/logstrip.h>
#include <ui/cmdline.h>
#include <train_calibrator.h>

#define LEN_MSG (64 * 4)
#define LEN_CMD 32

#define CONSOLE_LOG_COL 1
#define CONSOLE_LOG_LINE 29
#define CONSOLE_CMD_COL 2
#define CONSOLE_CMD_LINE 30
#define CONSOLE_SENSOR_COL 17
#define CONSOLE_SENSOR_LINE 6
#define CONSOLE_DUMP_LINE CONSOLE_CMD_LINE + 4
#define CONSOLE_DUMP_COL 1

#define CONSOLE_LANDMARK_LINE CONSOLE_DUMP_LINE
#define CONSOLE_LANDMARK_COL 1

#define NUM_TRIALS 10
#define MAX_TRIAL (NUM_TRIALS-1)

typedef struct {
	int trial;
	int dt[NUM_TRIALS];
} track_node_data;


static void reset_track_node_data(track_node_data *data) {
	data->trial = -1;
	for (int i = 0; i < NUM_TRIALS; i++) {
		data->dt[i] = -1;
	}
}

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

static void sensor_pic_def(
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

static void ui_init(a0state *state) {
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

	console_clear(state->con);
	console_move(state->con, 1, 1);
	console_printf(state->con, TRACK_TEMPLATE);
	console_flush(state->con);

	cmdline_clear(state->cmdline);

	return;
}

static void ui_time(a0state *state, int ticks) {
	console_move(state->con, 1, 9);
	console_erase_eol(state->con);
	int ms = TICK2MS(ticks);
	console_printf(state->con, "%d.%ds", ms / 1000, (ms / 100) % 10);
	console_flush(state->con);
}

static void ui_sensor(a0state *state, char module, int id) {
	for (int i = LEN_SENSOR_HIST - 1; i > 0; i--) {
		hist_mod[i] = hist_mod[i - 1];
		hist_id[i] = hist_id[i - 1];
	}
	hist_mod[0] = module;
	hist_id[0] = id;

	console_move(state->con, CONSOLE_SENSOR_LINE, CONSOLE_SENSOR_COL);
	console_erase_eol(state->con);

	int max_hist_idx = 0;

	for (int i = 0; (i < LEN_SENSOR_HIST) && hist_mod[i]; i++) {
		console_printf(state->con, "%c%d, ", hist_mod[i], hist_id[i]);
		if (i == LEN_SENSOR_HIST) {
			console_printf(state->con, "...");
		}

		max_hist_idx = i;
	}

	for (int i = max_hist_idx; i >= 0; i--) {
		sensor_pic_info spinfo = sensor_pic_info_table[hist_mod[i]-'A'][hist_id[i]];

		if (spinfo.dir != UNKNOWN) {
			console_move(state->con, spinfo.row, spinfo.col);

			switch (i) {
				case 0:
					console_effect(state->con, EFFECT_BRIGHT);
				case 1:
					console_effect(state->con, EFFECT_FG_CYAN);
					break;
				default:
					console_effect(state->con, EFFECT_FG_BLUE);
					break;
			}

			console_effect_reset(state->con);
			console_printf(state->con, "%s", direction_str[spinfo.dir]);
		}
	}
	console_flush(state->con);
}

static void ui_speed(a0state *state, int train, int speed) {
	logstrip_printf(state->logstrip, "set speed of train %d to %d", train, speed);
}

static void ui_reverse(a0state *state, int train) {
	logstrip_printf(state->logstrip, "reversed train %d", train);
}

static void ui_updateswitchstatus(console *c, char no, char pos) {
	int idx = train_switchno2i(no); // 0 based
	int statusrow = 2 + idx / 6;
	int statuscol = 14 + 5 * (idx % 6);
	char pos_name = train_switchpos_straight(pos) ? 'S' : 'C';

	console_move(c, statusrow, statuscol);
	console_effect(c, EFFECT_BRIGHT);
	console_effect(c, EFFECT_FG_YELLOW);
	console_printf(c, "%c", pos_name);
	console_effect_reset(c);

	switch_pic_info swinfo = switch_pic_info_table[idx];

	console_move(c, swinfo.row, swinfo.col);
	console_effect(c, EFFECT_BRIGHT);
	console_effect(c, EFFECT_FG_YELLOW);
	console_printf(c, "%c", (pos_name == 'S') ? swinfo.straight : swinfo.curved);
	console_effect_reset(c);
}

static void ui_switch(a0state *state, char no, char pos) {
	logstrip_printf(state->logstrip, "switched switch %d to %c", no, pos);
}

static void ui_switchall(a0state *state, char pos) {
	logstrip_printf(state->logstrip, "switched all switches to '%c'", pos);
	for (int i = 0; i < TRAIN_NUM_SWITCHADDR; i++) {
		ui_updateswitchstatus(state->con, train_switchi2no(i), pos);
	}
	console_flush(state->con);
}

static void ui_setup_demo_track(a0state *state) {
	logstrip_printf(state->logstrip, "adjusted all switches for demo");
}

static void ui_quit(a0state *state) {
	logstrip_printf(state->logstrip, "quitting...");
	console_move(state->con, CONSOLE_CMD_LINE + 1, 1);
	console_flush(state->con);
}

/*
 * Server
 */

static void handle_train_switch(a0state *state, int num, char pos) {
	char buf[10];
	train_switch(num, pos, state->tid_traincmdbuf);
	ui_switch(state, num, pos);
	sprintf(buf, "BR%d", num);
	track_node *branch = lookup_get(state->nodemap, buf);
	ASSERT(branch, "branch %s is null?", buf);
	branch->switch_dir = POS2DIR(pos);
}

static void handle_train_switch_all(a0state *state, char pos) {
	train_switchall(pos, state->tid_traincmdbuf);
	ui_switchall(state, pos);
	char buf[10];
	for (int i = 0; i < TRAIN_NUM_SWITCHADDR; i++) {
		sprintf(buf, "BR%d", train_switchi2no(i));
		track_node *branch = lookup_get(state->nodemap, buf);
		ASSERT(branch, "branch %s is null?", buf);
		branch->switch_dir = POS2DIR(pos);
	}
}

static lookup *ask_track(int tid_com2, track_node* data) {
	for (;;) {
		Putstr(COM2, "Track a or b?\n", tid_com2);
		char c = Getc(COM2, tid_com2);
		lookup *nodemap;
		switch (c) {
			case 'a':
				nodemap = init_tracka(data);
				populate_beta(nodemap);
				return nodemap;
			case 'b':
				nodemap = init_trackb(data);
				populate_beta(nodemap);
				return nodemap;
			default:
				Putstr(COM2, "fail\n", tid_com2);
				break;
		}
	}
	return NULL;
}

static void calib_sensor(void *s) {
	a0state *state = s;
	track_node *cur_node = state->cur_node;
	int tick = state->cur_tick;
	if (strcmp(cur_node->name, "E4") == 0) return;

	/*else if (strcmp(cur_node->name, "D5") == 0) {
		handle_train_switch(state, 153, 'C');
		handle_train_switch(state, 154, 'S');
		handle_train_switch(state, 155, 'C');
		handle_train_switch(state, 156, 'S');
	} else if (strcmp(cur_node->name, "C11") == 0) {
		handle_train_switch(state, 153, 'S');
		handle_train_switch(state, 154, 'C');
		handle_train_switch(state, 155, 'S');
		handle_train_switch(state, 156, 'C');
	}*/

	int dist = find_dist(state->last_node, cur_node, 0, 2); // tolerate at most 1 missed sensors

	// get a reference to the sensor track node data
	track_node_data *data;
	if (cur_node->data) {
		data = (track_node_data*) cur_node->data;
	} else {
		cur_node->data = malloc(sizeof(track_node_data));
		data = (track_node_data*) cur_node->data;
		reset_track_node_data(data);
	}

	if (data->trial == -1) goto exit;

	if (data->trial > MAX_TRIAL) {
		state->console_dump_line = CONSOLE_DUMP_LINE;
		goto exit;
	}

	data->dt[data->trial] = tick - state->last_tick;

	if (data->trial < MAX_TRIAL) goto exit;

	console_move(state->con, state->console_dump_line++, CONSOLE_DUMP_COL);
	console_erase_eol(state->con);
	console_printf(state->con, "%s\t%s\t%d", state->last_node->name, cur_node->name, dist);

	for (int trial = 0; trial <= MAX_TRIAL; trial++) {
		console_printf(state->con, "\t%d", data->dt[trial]);
	}

	console_flush(state->con);

	exit:
	data->trial++;
}

static void print_landmark(void* s) {
	a0state *state = s;
	track_node *node = state->cur_node;
	int tick = state->cur_tick;

	if (state->last_node && state->last_node != node) {
		track_node *curnode = state->last_node;
		track_edge *curedge = find_forward(curnode);
		ASSERT(node, "node is null!!");
		int totaldist = find_dist(state->last_node, node, 0, 1);
		if (totaldist < 0) return;
		fixed tref = state->train_desc[37].tref[(int) state->train_speed[37]]; // hardcoded
		if (tref < 0) return;
		fixed total_beta = fixed_new(0);

		console_move(state->con, CONSOLE_LANDMARK_LINE + state->linecnt++ % 20, CONSOLE_LANDMARK_COL);
		console_erase_eol(state->con);

		while (curnode != node) {
			ASSERT(curedge, "curedge is null. finding %s to %s, curnode:%s total_beta: %F",
					state->last_node->name, node->name, curnode->name, total_beta);
			ASSERT(curedge->beta != fixed_new(-1), "edge %s->%s beta is uninitialized.", PREV_EDGE(curedge)->name, curedge->dest->name);
			total_beta = fixed_add(total_beta, curedge->beta);
			curnode = curedge->dest;
			curedge = find_forward(curnode);
		}

		fixed expected_time = fixed_mul(total_beta, tref);
		fixed actual_time = fixed_new(tick - state->last_tick);

		console_printf(state->con, "%s,%s,%F,%F,%F", state->last_node->name, node->name, expected_time, actual_time, total_beta);
	}

	console_flush(state->con);
}

// static void measure_speed_from_sensor(a0state *state, track_node *sensor, msg_sensor *sensor_msg) {
// 	if (sensor_msg->on == FALSE) {
// 		console_move(state->con, state->console_dump_line++, CONSOLE_DUMP_COL);
// 		int dx = 50;
// 		int dt = TICK2MS(sensor_msg->ticks - state->last_tick);
// 		console_printf(state->con, "dx: %d, dt: %d, dx/dt: %d\n", dx, dt, (100000 * dx) / dt);
// 		console_flush(state->con);
// 	}
// }

static void handle_sensor(a0state *state, char msg[]) {
	msg_sensor *sensor_msg = (msg_sensor*) msg;
	char modname[8];
	sprintf(modname, "%c%d", sensor_msg->module, sensor_msg->id);
	track_node *sensor = lookup_get(state->nodemap, modname);
	// measure_speed_from_sensor(state, sensor, sensor_msg);
	state->cur_node = sensor;
	state->cur_tick = sensor_msg->ticks;
	ui_sensor(state, sensor_msg->module, sensor_msg->id);
	dumbbus_dispatch(state->sensor_listeners, state);
	state->last_tick = state->cur_tick;
	state->last_node = sensor;
}

#define ACCEPT(a) { \
	if (*c++ != a) { \
		goto badcmd; \
	} \
}

static void handle_command(void* s, char *cmd, int size) {
	a0state *state = s;
	int quit = FALSE;
	char *c = cmd;
	cmdline_clear(state->cmdline);

	if (!size) goto badcmd;

	switch (*c++) {
		case 'c': {
			ACCEPT('a');
			ACCEPT('l');
			ACCEPT(' ');
			int train = strgetui(&c);
			if (!train_goodtrain(train)) goto badcmd;
			start_train_calibration(state, train);
			break;
		}
		case 'd': {
			handle_train_switch_all(state, 'C');
			handle_train_switch(state, 9, 'S');
			handle_train_switch(state, 10, 'S');
			handle_train_switch(state, 15, 'S');
			handle_train_switch(state, 16, 'S');
			handle_train_switch(state, 14, 'S');
			train_solenoidoff(state->tid_traincmdbuf);
			ui_setup_demo_track(state);
			break;
		}
		case 't': { // set train speed_avg (tr # #)
			ACCEPT('r');
			ACCEPT(' ');
			int train = strgetui(&c);
			if (!train_goodtrain(train)) goto badcmd;
			ACCEPT(' ');
			int speed = strgetui(&c);
			if (!train_goodspeed(speed)) goto badcmd;
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
			int speed_avg = state->train_speed[train];
			ui_reverse(state, train);
			train_speed(train, 0, state->tid_traincmdbuf);
			train_reverse(train, state->tid_traincmdbuf);
			train_speed(train, speed_avg, state->tid_traincmdbuf);
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
			if (switchno == '*') {
				handle_train_switch_all(state, pos);
			} else {
				handle_train_switch(state, switchno, pos);
			}
			train_solenoidoff(state->tid_traincmdbuf);
			break;
		}
		case 'q': { // quit kernel
			quit = TRUE;
			break;
		}
		default: {
			goto badcmd;
			break;
		}
	}

	if (quit) {
		// @TODO: pull this out with a "finally"
		TRAIN_FOREACH(t) {
			if (state->train_speed[t]) {
				train_speed(t, 0, state->tid_traincmdbuf);
			}
		}
		ui_quit(state);
		train_stop(state->tid_traincmdbuf);
		Flush(state->tid_com1);
		Flush(state->tid_com2);
		ExitKernel(0);
		ASSERT(0, "ExitKernel returned!");
	}

	return;
	badcmd:
	logstrip_printf(state->logstrip, "invalid command: \"%s\"", cmd);
}

static void handle_comin(a0state *state, char msg[]) {
	msg_comin *comin = (msg_comin*) msg;
	ASSERT(comin->channel == COM2, "no handler for channel %d", comin->channel);
	cmdline_handleinput(state->cmdline, comin->c);
}

static void handle_time(a0state *state, char msg[]) {
	msg_time *time = (msg_time*) msg;
	ui_time(state, time->ticks);
}

void a0() {
	a0state state;
	console con;
	cmdline cmd;
	dumbbus sensor_listeners;

	state.tid_time = WhoIs(NAME_TIMESERVER);
	state.tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	state.tid_com2 = WhoIs(NAME_IOSERVER_COM2);
	ASSERT(state.tid_com2 >= 0, "invalid com2 server: %d", state.tid_com2);

	// ui
	state.con = &con;
	console_create(state.con, state.tid_com2);
	state.logstrip = logstrip_create(CONSOLE_LOG_LINE, CONSOLE_LOG_COL, state.con);
	state.cmdline = &cmd;
	cmdline_create(&cmd, CONSOLE_CMD_LINE, CONSOLE_CMD_COL, state.con, handle_command, &state);

	// sensor listeners
	state.sensor_listeners = &sensor_listeners;
	dumbbus_init(&sensor_listeners);
	// dumbbus_register(&sensor_listeners, &calib_sensor);
	dumbbus_register(&sensor_listeners, &print_landmark);

	state.tid_traincmdbuf = traincmdbuffer_new();
	traincmdrunner_new();
	state.console_dump_line = CONSOLE_DUMP_LINE;
	TRAIN_FOREACH(i) {
		state.train_speed[i] = 0;
		TRAIN_FOREACH_SPEED(speed) {
			state.train_desc[i].tref[speed] = fixed_new(-1);
		}
	}
	track_node *track_data = malloc(sizeof(track_node) * TRACK_MAX);
	state.nodemap = ask_track(state.tid_com2, track_data);
	state.last_tick = 0;
	state.last_node = NULL;
	state.trial = 0;

	state.linecnt = 0;

	ui_init(&state);

	MEMCHECK();

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
