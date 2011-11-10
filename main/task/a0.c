#include <train.h>
#include <syscall.h>
#include <uconst.h>
#include <string.h>
#include <stdio.h>
#include <console.h>
#include <lookup.h>
#include <fixed.h>
#include <dumbbus.h>
#include <server/sensornotifier.h>
#include <server/comnotifier.h>
#include <server/timenotifier.h>
#include <task/a0.h>
#include <ui/a0_track_template.h>
#include <ui/logstrip.h>
#include <ui/cmdline.h>
#include <ui/logdisplay.h>
#include <train_calibrator.h>
#include <train_stopper.h>

#define LEN_MSG (64 * 4)
#define LEN_CMD 32

#define CONSOLE_DUMP_LINE (CONSOLE_CMD_LINE + 4)
#define CONSOLE_DUMP_COL 1

#define CONSOLE_EXTIME_LINE 8
#define CONSOLE_EXTIME_COL 65
#define CONSOLE_EXTIME_SIZE 10

#define CONSOLE_LANDMARK_LINE (CONSOLE_EXTIME_LINE + CONSOLE_EXTIME_SIZE + 3)
#define CONSOLE_LANDMARK_COL CONSOLE_EXTIME_COL

#define NUM_TRIALS 10
#define MAX_TRIAL (NUM_TRIALS - 1)

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

static void ui_init(a0state *state) {
	// init sensor hist
	for (int i = 0; i < LEN_SENSOR_HIST; i++) {
		hist_mod[i] = 0;
		hist_id[i] = 0;
	}

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

	switch_pic_info *swinfo = get_switch_pic_info(idx);

	console_move(c, swinfo->row, swinfo->col);
	console_effect(c, EFFECT_BRIGHT);
	console_effect(c, EFFECT_FG_YELLOW);
	console_printf(c, "%c", (pos_name == 'S') ? swinfo->straight : swinfo->curved);
	console_effect_reset(c);
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
	char buf[1024], *p = buf;
	for (int i = 0; (i < LEN_SENSOR_HIST) && hist_mod[i]; i++) {
		p += sprintf(p, "%c%d, ", hist_mod[i], hist_id[i]);
		if (i == LEN_SENSOR_HIST) {
			p += sprintf(p, "...");
		}
		max_hist_idx = i;
	}
	logstrip_printf(state->sensorlog, buf);

	for (int i = max_hist_idx; i >= 0; i--) {
		sensor_pic_info *spinfo = get_sensor_pic_info(hist_mod[i], hist_id[i]);

		if (spinfo->dir != UNKNOWN) {
			console_move(state->con, spinfo->row, spinfo->col);

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

			console_printf(state->con, "%s", spinfo->dir_str);
			console_effect_reset(state->con);
		}
	}
	console_flush(state->con);
}


static void ui_speed(a0state *state, int train, int speed) {
	logstrip_printf(state->cmdlog, "set speed of train %d to %d", train, speed);
}

static void ui_reverse(a0state *state, int train) {
	logstrip_printf(state->cmdlog, "reversed train %d", train);
}


static void ui_switch(a0state *state, char no, char pos) {
	logstrip_printf(state->cmdlog, "switched switch %d to %c", no, pos);
	ui_updateswitchstatus(state->con, no, pos);
	console_flush(state->con);
}

static void ui_switchall(a0state *state, char pos) {
	logstrip_printf(state->cmdlog, "switched all switches to '%c'", pos);
	console_flush(state->con);
}

void ui_set_track(a0state *state, int s[], int ns, int c[], int nc) {
	for (int i = 0; i < ns; i++) {
		ui_updateswitchstatus(state->con, s[i], 's');
	}
	for (int i = 0; i < nc; i++) {
		ui_updateswitchstatus(state->con, c[i], 'c');
	}
}

static void ui_setup_demo_track(a0state *state) {
	logstrip_printf(state->cmdlog, "adjusted all switches for demo");
}

static void ui_quit(a0state *state) {
	logstrip_printf(state->cmdlog, "quitting...");
	console_move(state->con, CONSOLE_CMD_LINE + 1, 1);
	console_flush(state->con);
}

/*
 * Server
 */

static void handle_train_switch_all(a0state *state, char pos) {
	engineer *eng = state->eng;
	int all[] = {
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		17, 18, 153, 154, 155, 156
	};
	int count = sizeof(all) / sizeof(int);
	if (train_switchpos_straight(pos)) {
		engineer_set_track(eng, all, count, NULL, 0);
		ui_set_track(state, all, count, NULL, 0);
	} else {
		engineer_set_track(eng, NULL, 0, all, count);
		ui_set_track(state, NULL, 0, all, count);
	}
	ui_switchall(state, pos);
}

static track ask_track(a0state *state) {
	for (;;) {
		Putstr(COM2, "Track a or b?\n", state->tid_com2);
		char c = Getc(COM2, state->tid_com2);
		switch (c) {
			case 'a':
				return TRACK_A;
			case 'b':
				return TRACK_B;
			default:
				Putstr(COM2, "fail\n", state->tid_com2);
				break;
		}
	}
	return '\0';
}

static void update_train_velocity(void *s) {
	// a0state *state = s;
	// engineer *eng = state->eng;
	// int train_no = state->cur_train;

	// if (!TRAIN_GOODNO(train_no)) return;

	// track_node *sensor = state->cur_node;
	// track_node *last_sensor = state->last_node;

	// int dx = find_dist(last_sensor, sensor, 0, 2);
	// if (dx <= 0) return;
	// int dt = state->cur_tick - state->last_tick;
	// if (dt <= 0) return;
	// fixed v = fixed_div(fixed_new(dx), fixed_new(dt));

	// fixed beta = beta_sum(last_sensor, sensor);
	// fixed speed_idx = engineer_get_speedidx(eng, train_no);
	// fixed tref = engineer_get_tref(eng, train_no, speed_idx);
	// fixed dt_ = fixed_mul(beta, tref);
	// if (dt_ <= 0) return;
	// fixed v_ = fixed_div(fixed_new(dx), fixed_new(dt_));

	// logstrip_printf(state->cmdlog, "measured v: %F, stored v: %F", v, v_);

	// engineer_set_velocity(eng, train_no, v);
}

static void calib_sensor(void *s) {
	a0state *state = s;
	track_node *cur_node = state->cur_node;
	int tick = state->cur_tick;
	if (strcmp(cur_node->name, "E4") == 0) return;

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

	if (data->trial == -1 || data->trial > MAX_TRIAL) {
		goto exit;
	}

	data->dt[data->trial] = tick - state->last_tick;

	if (data->trial < MAX_TRIAL) goto exit;

	logdisplay_printf(state->console_dump, "%s\t%s\t%d", state->last_node->name, cur_node->name, dist);

	for (int trial = 0; trial <= MAX_TRIAL; trial++) {
		logdisplay_printf(state->console_dump, "\t%d", data->dt[trial]);
	}

	logdisplay_flushline(state->console_dump);

	exit: data->trial++;
}

static void print_landmark(void* s) {
	a0state *state = s;
	engineer *eng = state->eng;

	int train_no = state->cur_train;
	if (!TRAIN_GOODNO(train_no)) return;

	if (train_no == -1) {
		logstrip_printf(state->landmark_display, "No train is calibrated.");
		return;
	}

	int speed = engineer_get_speed(eng, train_no);
	int speed_idx = engineer_get_speedidx(eng, train_no);
	int tref = engineer_get_tref(eng, train_no, speed_idx);
	if (tref == -1) {
		logstrip_printf(state->landmark_display, "I have no calibration data for train %d at speed %d.", train_no, speed);
		return;
	}

	track_node *last_node = state->last_node;
	ASSERTNOTNULL(last_node);

	int dt = Time(state->tid_time) - state->last_tick;
	int est_dist_cm = calc_distance_after(last_node, dt, tref);
	char *direction_str;
	switch (engineer_train_get_dir(eng, train_no)) {
		case TRAIN_FORWARD:
			direction_str = "forward";
			break;
		case TRAIN_BACKWARD:
			direction_str = "backward";
			break;
		default:
			direction_str = "unknown";
			break;
	}

	if (est_dist_cm >= 0) {
		logstrip_printf(state->landmark_display, "Train %2d (%8s) is %4dcm ahead of %s, dir: %s.", train_no, direction_str, est_dist_cm, last_node->name, direction_str);
	} else {
		logstrip_printf(state->landmark_display, "Train %2d (%8s): I don't know any path ahead of %d", train_no, direction_str, last_node->name);
	}
	//	logstrip_printf(state->landmark_display, "dist: %d tref: %5d beta: %10F tick_diff: %10F", expected_edge->dist, tref, beta, tick_diff);
}

static void print_expected_time(void* s) {
	a0state *state = s;
	engineer *eng = state->eng;
	int train_no = state->cur_train;
	if (!TRAIN_GOODNO(train_no)) return; // no train calibrated

	int speed_idx = engineer_get_speedidx(eng, train_no);
	int tref = engineer_get_tref(eng, train_no, speed_idx);
	if (tref == -1) return; // no calibration data exists at this speed

	track_node *sensor = state->cur_node;
	track_node *last_sensor = state->last_node;

	if (last_sensor && find_dist(last_sensor, sensor, 0, 1) > 0) {
		fixed total_beta = beta_sum(last_sensor, sensor);
		if (fixed_iszero(total_beta)) return; // unknown path

		fixed expected_time = fixed_mul(fixed_new(TICK2MS(tref)), total_beta);
		fixed actual_time = fixed_new(TICK2MS(state->cur_tick - state->last_tick));

		logdisplay_printf(state->expected_time_display,
			"Edge %3s->%-3s\tExpected:%10Fms  Actual:%10Fms  Diff:%10Fms",
			last_sensor->name,
			sensor->name,
			expected_time,
			actual_time,
			fixed_sub(actual_time, expected_time)
		);
		logdisplay_flushline(state->expected_time_display);
	}
}

static void handle_sensor(a0state *state, char msg[]) {
	engineer *eng = state->eng;
	msg_sensor *m = (msg_sensor*) msg;
	track_node *sensor = engineer_get_tracknode(eng, m->module, m->id);

	/*if (strcmp(sensor->name, "D5") == 0) {
		engineer_set_switch(eng, 153, 'C', FALSE);
		engineer_set_switch(eng, 154, 'S', FALSE);
		engineer_set_switch(eng, 155, 'C', FALSE);
		engineer_set_switch(eng, 156, 'S', TRUE);
	} else if (strcmp(sensor->name, "C11") == 0) {
		engineer_set_switch(eng, 153, 'S', FALSE);
		engineer_set_switch(eng, 154, 'C', FALSE);
		engineer_set_switch(eng, 155, 'S', FALSE);
		engineer_set_switch(eng, 156, 'C', TRUE);
	}*/

	ui_sensor(state, m->module[0], m->id);
	state->last_node = state->cur_node;
	state->cur_node = sensor;
	state->last_tick = state->cur_tick;
	state->cur_tick = m->ticks;
	dumbbus_dispatch(state->sensor_listeners, state);
}

static void handle_setup_demotrack(a0state *state) {
	engineer *eng = state->eng;
	int s[] = {9, 10, 14, 15, 16,};
	int c[] = {1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 17, 18, 153, 154, 155, 156};
	int ns = sizeof(s) / sizeof(int);
	int nc = sizeof(c) / sizeof(int);
	engineer_set_track(eng, s, ns, c, nc);
	// update ui
	ui_set_track(state, s, ns, c, nc);
	ui_setup_demo_track(state);
}

#define ACCEPT(a) { \
	if (*c++ != a) { \
		goto badcmd; \
	} \
}

static void handle_command(void* s, char *cmd, int size) {
	a0state *state = s;
	engineer *eng = state->eng;
	int quit = FALSE;
	char *c = cmd;
	cmdline_clear(state->cmdline);

	if (!size)
		goto badcmd;

	switch (*c++) {
		case 'c': {
			ACCEPT('a');
			ACCEPT('l');
			ACCEPT(' ');
			int train = strgetui(&c);
			if (!train_goodtrain(train)) goto badcmd;
			ACCEPT(' ');
			int min = strgetui(&c);
			if (!calib_goodmin(min)) goto badcmd;
			ACCEPT(' ');
			int max = strgetui(&c);
			if (!calib_goodmax(max)) goto badcmd;
			if (min > max) goto badcmd;
			logstrip_printf(state->cmdlog, "calibrating train %d from speed %d to %d", train, min, max);
			calibrate_train(state, train, min, max);
			break;
		}
		case 'd': {
			handle_setup_demotrack(state);
			break;
		}
		case 't': { // set train speed(tr # #)
			ACCEPT('r');
			ACCEPT(' ');
			int train = strgetui(&c);
			if (!train_goodtrain(train)) goto badcmd;
			ACCEPT(' ');
			int speed = strgetui(&c);
			if (!train_goodspeed(speed)) goto badcmd;
			ui_speed(state, train, speed);
			engineer_set_speed(eng, train, speed);
			break;
		}
		case 'r': { // reverse train (rv #+)
			ACCEPT('v');
			ACCEPT(' ');
			int train = strgetui(&c);
			if (!train_goodtrain(train)) goto badcmd;
			ui_reverse(state, train);
			engineer_reverse(eng, train);
			break;
		}
		case 's': {
			if (*c == 't') { // set switch position (st #+ [a-zA-Z]+[0-9]+ #+)
				ACCEPT('t');
				ACCEPT(' ');
				int train = strgetui(&c);
				if (!train_goodtrain(train)) goto badcmd;
				ACCEPT(' ');
				char type[4];
				uint len = strgetw(c, type, 4);
				if (len == 0) goto badcmd;
				c += len;
				int id = strgetui(&c);
				ACCEPT(' ');
				int dist_cm = strgetui(&c);
				train_stopper_setup(state, train, type, id, dist_cm);
			} else if (*c == 'w') { // set switch position (sw #+ [cCsS])
				ACCEPT('w');
				ACCEPT(' ');
				char id;
				if (*c == '*') {
					id = *c++;
				} else {
					id = strgetui(&c);
					if (!train_goodswitch(id)) goto badcmd;
				}
				ACCEPT(' ');
				char pos = *c++;
				if (!train_goodswitchpos(pos))
					goto badcmd;
				if (id == '*') {
					handle_train_switch_all(state, pos);
				} else {
					ui_switch(state, id, pos);
					engineer_set_switch(eng, id, pos, TRUE);
				}
			} else {
				goto badcmd;
			}
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
		engineer_destroy(eng);
		ui_quit(state);
		Flush(state->tid_com1);
		Flush(state->tid_com2);
		ExitKernel(0);
		ASSERT(0, "ExitKernel returned!");
	}

	return;
	badcmd:
	logstrip_printf(state->cmdlog, "invalid command: \"%s\"", cmd);
}

static void handle_comin(a0state *state, char msg[]) {
	msg_comin *comin = (msg_comin*) msg;
	ASSERT(comin->channel == COM2, "no handler for channel %d", comin->channel);
	cmdline_handleinput(state->cmdline, comin->c);
}

static void handle_time(a0state *state, char msg[]) {
	msg_time *time = (msg_time*) msg;
	ui_time(state, time->ticks);
	dumbbus_dispatch(state->time_listeners, state);
}

void a0() {
	a0state state;
	console con;
	cmdline cmd;

	state.tid_time = WhoIs(NAME_TIMESERVER);
	state.tid_com1 = WhoIs(NAME_IOSERVER_COM1);
	state.tid_com2 = WhoIs(NAME_IOSERVER_COM2);
	ASSERT(state.tid_com2 >= 0, "invalid com2 server: %d", state.tid_com2);

	// ui
	state.con = &con;
	console_create(state.con, state.tid_com2);
	state.cmdlog = logstrip_create(CONSOLE_LOG_LINE, CONSOLE_LOG_COL, state.con);
	state.cmdline = &cmd;
	cmdline_create(&cmd, CONSOLE_CMD_LINE, CONSOLE_CMD_COL, state.con, handle_command, &state);
	state.sensorlog = logstrip_create(CONSOLE_SENSOR_LINE, CONSOLE_SENSOR_COL, state.con);
	state.console_dump = logdisplay_new(&con, CONSOLE_DUMP_LINE, CONSOLE_DUMP_COL, 20, ROUNDROBIN);
	state.expected_time_display = logdisplay_new(&con, CONSOLE_EXTIME_LINE, CONSOLE_EXTIME_COL, CONSOLE_EXTIME_SIZE, SCROLLING);
	state.landmark_display = logstrip_create(CONSOLE_LANDMARK_LINE, CONSOLE_LANDMARK_COL, &con);

	// sensor listeners
	state.sensor_listeners = dumbbus_new();
	dumbbus_register(state.sensor_listeners, &update_train_velocity);
	// dumbbus_register(&sensor_listeners, &calib_sensor);
	dumbbus_register(state.sensor_listeners, &print_expected_time);

	// time listeners
	state.time_listeners = dumbbus_new();
	dumbbus_register(state.time_listeners, &print_landmark);

	track track = ask_track(&state);
	init_track_template(track, state.con);
	state.eng = engineer_new(track == TRACK_A ? 'a' : 'b');
	state.cur_train = -1;
	state.last_tick = 0;
	state.last_node = NULL;
	state.trial = 0;

	calibrator_init();

	state.linecnt = 0;

	ui_init(&state);

	MEMCHECK();

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
