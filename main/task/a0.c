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
#include <ui/timedisplay.h>
#include <train_calibrator.h>
#include <train_stopper.h>
#include <server/buffertask.h>
#include <server/courier.h>
#include <server/publisher.h>

#define LEN_MSG (64 * 4)

#define CONSOLE_DUMP_LINE (CONSOLE_CMD_LINE + 4)
#define CONSOLE_DUMP_COL 1
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
}

static void ui_time(void* s) {
	a0state *state = s;
	timedisplay_update(state->timedisplay, state->timestamp);
}

static void ui_updateswitchstatus(a0state *state, char no, char pos) {
	track_template_updateswitch(state->template, no, pos);
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
	console_flush(state->con);

	track_template_updatesensor(state->template, module, id, 0);
}


static void ui_speed(a0state *state, int train, int speed) {
	logstrip_printf(state->cmdlog, "set speed of train %d to %d", train, speed);
}

static void ui_reverse(a0state *state, int train) {
	logstrip_printf(state->cmdlog, "reversed train %d", train);
}

static void ui_switch(a0state *state, char no, char pos) {
	logstrip_printf(state->cmdlog, "switched switch %d to %c", no, pos);
	ui_updateswitchstatus(state, no, pos);
	console_flush(state->con);
}

static void ui_switchall(a0state *state, char pos) {
	logstrip_printf(state->cmdlog, "switched all switches to '%c'", pos);
	console_flush(state->con);
}

void ui_set_track(a0state *state, int s[], int ns, int c[], int nc) {
	for (int i = 0; i < ns; i++) {
		ui_updateswitchstatus(state, s[i], 's');
	}
	for (int i = 0; i < nc; i++) {
		ui_updateswitchstatus(state, c[i], 'c');
	}
}

static void ui_setup_demo_track(a0state *state) {
	logstrip_printf(state->cmdlog, "adjusted all switches for demo");
}

static void ui_quit(a0state *state) {
	logstrip_printf(state->cmdlog, "quitting...");
	console_move(state->con, CONSOLE_CMD_LINE + 1, 1);
	console_flush(state->con);
	console_flushcom(state->con);
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
	if (track_switchpos_straight(pos)) {
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
		console_clear(state->con);
		console_printf(state->con, "Track a or b?\n");
		console_flush(state->con);
		char c = Getc(COM2, state->con->tid_console);
		switch (c) {
			case 'a':
				return TRACK_A;
			case 'b':
				return TRACK_B;
			default:
				console_printf(state->con, "fail\n");
				console_flush(state->con);
				break;
		}
	}
	ASSERT(0, "unreachable"); // unreachable
	return 0;
}

#define STOP_INIT 0
#define STOP_UP2SPEED1 1
#define STOP_UP2SPEED2 2
#define STOP_STOPPING 3
#define STOP_REVERSING 4

struct {
	int state;
	int speed;
} csdstate;

static void init_csdstate() {
	csdstate.state = STOP_INIT;
	csdstate.speed = 4;
}

// static void calib_stopdist(void* s) {
// 	a0state *state = s;
// 	engineer *eng = state->eng;
// 	int train_no = state->cur_train;
// 	if (train_no < 0) return;
// 	track_node *cur_sensor = state->cur_sensor;
// 	track_node *e8 = engineer_get_tracknode(eng, "E", 8);
// 	switch (csdstate.state) {
// 		case STOP_INIT:
// 			engineer_set_switch(eng, 11, 'c', TRUE);
// 			engineer_set_speed(eng, train_no, csdstate.speed);
// 			csdstate.state = STOP_UP2SPEED1;
// 			break;
// 		case STOP_UP2SPEED1:
// 			if (cur_sensor == e8) {
// 				csdstate.state = STOP_STOPPING;
// 			}
// 			break;
// 		case STOP_UP2SPEED2:
// 			if (cur_sensor == e8) {
// 				csdstate.state = STOP_STOPPING;
// 			}
// 			break;
// 		case STOP_STOPPING:
// 			if (cur_sensor == e8) {
// 				engineer_set_speed(eng, train_no, 0);
// 				engineer_set_switch(eng, 11, 's', TRUE);
// 				engineer_train_pause(eng, train_no, MS2TICK(5000));
// 				if (csdstate.speed == 14) {
// 					csdstate.speed = 4;
// 				} else {
// 					csdstate.speed += 1;
// 				}
// 				engineer_reverse(eng, train_no);
// 				engineer_set_speed(eng, train_no, 14);
// 				csdstate.state = STOP_REVERSING;
// 			}
// 			break;
// 		case STOP_REVERSING:
// 			if (strcmp(cur_sensor->name, "E7") == 0) {
// 				engineer_set_switch(eng, 11, 'c', TRUE);
// 				engineer_reverse(eng, train_no);
// 				engineer_set_speed(eng, train_no, csdstate.speed);
// 				csdstate.state = STOP_UP2SPEED1;
// 			}
// 			break;
// 	}
// }

struct {
	int state;
	int count;
	int dx;
	int dt;
	track_node *start;
	track_node *last_sensor;
	int last_timestamp;
	int v_i_x;
	int v_i_t;
} j;

#define JERK_STABILIZE_NUM_SENSORS 10

static void init_jerk() {
	j.state = 0;
}

// static void jerk(a0state *state, track_node *sensor, int timestamp) {
// 	engineer *eng = state->eng;
// 	track_node *last_sensor = j.last_sensor;
// 	int dx = track_distance(last_sensor, sensor);
// 	int dt = TICK2MS(timestamp - j.last_timestamp);
// 	j.last_sensor = sensor;
// 	j.last_timestamp = timestamp;

// 	switch (j.state) {
// 		case 0: // get to v_i
// 			logstrip_printf(state->cmdlog, "getting to v_i");
// 			engineer_set_speed(eng, 38, 8);
// 			j.state = 1;
// 			j.count = 0;
// 			break;
// 		case 1: // stabilize v_i
// 			logstrip_printf(state->cmdlog, "stabilizing v_i");
// 			j.count++;
// 			if (j.count == JERK_STABILIZE_NUM_SENSORS) {
// 				j.state = 2;
// 			}
// 			break;
// 		case 2: // get to v_f
// 			logstrip_printf(state->cmdlog, "getting to v_f");
// 			engineer_set_speed(eng, 38, 14);
// 			j.state = 3;
// 			j.count = 0;
// 			j.dx = 0;
// 			j.dt = 0;
// 			j.v_i_x = dx;
// 			j.v_i_t = dt;
// 			j.start = sensor;
// 			break;
// 		case 3: // stabilize v_f
// 			logstrip_printf(state->cmdlog, "stabilizing v_f");
// 			j.dx += dx;
// 			j.dt += dt;
// 			j.count++;
// 			if (j.count == JERK_STABILIZE_NUM_SENSORS) {
// 				// "dx/dt = %d/%d from %s to %s, v_i = %d/%d, v_f = %d/%d"
// 				logdisplay_printf(
// 					state->log,
// 					"%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d",
// 					j.start->name, sensor->name,
// 					j.dx, j.dt,
// 					j.v_i_x, j.v_i_t,
// 					dx, dt
// 				);
// 				logdisplay_flushline(state->log);
// 				j.state = 0;
// 			}
// 			break;
// 	}
// }

static void handle_sensor(a0state *state, char rawmsg[]) {
	engineer *eng = state->eng;
	msg_sensor *m = (msg_sensor*) rawmsg;
	if (m->state == OFF) return;
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

	state->last_sensor = state->cur_sensor;
	state->cur_sensor = sensor;

	// jerk(state, sensor, m->timestamp); (THIS CODE HANGS THE CODE)
	ui_sensor(state, m->module[0], m->id);
	dumbbus_dispatch(state->sensor_bus, state);
	engineer_onsensor(eng, rawmsg);
}

static void printloc(void* s) {
	a0state *state = s;
	engineer *eng = state->eng;

	int train_no = state->cur_train;
	if (train_no < 0) return;

	location loc;
	engineer_get_loc(eng, train_no, &loc);
	if (location_isundef(&loc)) {
		logstrip_printf(state->trainloc, "waiting for velocity to settle...");
		return;
	}

	// print
	char *direction_str;
	switch (engineer_train_get_dir(eng, train_no)) {
		case TRAIN_FORWARD:
			direction_str = "forward";
			break;
		case TRAIN_BACKWARD:
			direction_str = "backward";
			break;
		default:
			direction_str = "somewhere";
			break;
	}

	logstrip_printf(state->trainloc,
		"%-5s + %Fcm heading %s",
		location_isundef(&loc) ? "?" : loc.edge->src->name,
		fixed_div(loc.offset, fixed_new(10)),
		direction_str
	);
}

static void handle_setup_demotrack(a0state *state) {
	engineer *eng = state->eng;
	int s[] = {6, 9, 10, 14, 15, 16,};
	int c[] = {1, 2, 3, 4, 5, 7, 8, 11, 12, 13, 17, 18, 153, 154, 155, 156};
	int ns = sizeof(s) / sizeof(int);
	int nc = sizeof(c) / sizeof(int);
	engineer_set_track(eng, s, ns, c, nc);
	// update ui
	ui_set_track(state, s, ns, c, nc);
	ui_setup_demo_track(state);
}

static void tick_engineer(void* s) {
	a0state *state = s;
	engineer_ontick(state->eng);
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

	if (!size) goto badcmd;

	switch (*c++) {
		case 'c': {
			ACCEPT('a');
			ACCEPT('l');
			ACCEPT(' ');
			int train = strgetui(&c);
			if (!train_goodtrain(train)) goto badcmd;
			// ACCEPT(' ');
			// int min = strgetui(&c);
			// if (!calib_goodmin(min)) goto badcmd;
			// ACCEPT(' ');
			// int max = strgetui(&c);
			// if (!calib_goodmax(max)) goto badcmd;
			// if (min > max) goto badcmd;
			ACCEPT('\0');
			logstrip_printf(state->cmdlog, "calibrating train %d", train);
			// logstrip_printf(state->cmdlog, "calibrating train %d from speed %d to %d", train, min, max);
			int min = 8;
			int max = 12;
			calibrate_train(state, train, min, max);
			break;
		}
		case 'd': {
			ACCEPT('\0');
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
			ACCEPT('\0');
			ui_speed(state, train, speed);
			engineer_set_speed(eng, train, speed);
			break;
		}
		case 'r': { // reverse train (rv #+)
			ACCEPT('v');
			ACCEPT(' ');
			int train = strgetui(&c);
			if (!train_goodtrain(train)) goto badcmd;
			ACCEPT('\0');
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
				ACCEPT('\0');
				train_stopper_setup(state, train, type, id, 10 * dist_cm);
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
				if (!track_switchpos_isgood(pos)) goto badcmd;
				ACCEPT('\0');
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
			ACCEPT('\0');
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
		ExitKernel(0);
	}

	return;
	badcmd:
	logstrip_printf(state->cmdlog, "bad command: \"%s\"", cmd);
}

static void handle_comin(a0state *state, char msg[]) {
	msg_comin *comin = (msg_comin*) msg;
	ASSERT(comin->channel == COM2, "no handler for channel %d", comin->channel);
	cmdline_handleinput(state->cmdline, comin->c);
}

static void handle_time(a0state *state, char msg[], int tid) {
	msg_time *time = (msg_time*) msg;
	state->timestamp = time->timestamp;
	if (tid == state->tid_refresh) {
		dumbbus_dispatch(state->bus10hz, state);
	} else if (tid == state->tid_simstep) {
		dumbbus_dispatch(state->simbus, state);
	} else {
		ASSERT(0, "time message from unknown task");
	}
}

void a0() {
	a0state state;

	state.tid_time = WhoIs(NAME_TIMESERVER);

	// ui
	state.con = console_new(COM2);
	track track = ask_track(&state);
	state.template = track_template_new(state.con, track);
	state.cmdlog = logstrip_new(state.con, CONSOLE_LOG_LINE, CONSOLE_LOG_COL);
	state.cmdline = cmdline_new(state.con, CONSOLE_CMD_LINE, CONSOLE_CMD_COL, handle_command, &state);
	state.sensorlog = logstrip_new(state.con, CONSOLE_SENSOR_LINE, CONSOLE_SENSOR_COL);
	state.log = logdisplay_new(state.con, CONSOLE_DUMP_LINE, CONSOLE_DUMP_COL, 20, 40, SCROLLING);
	state.timedisplay = timedisplay_new(state.con, 1, 9);
	state.trainloc = logstrip_new(state.con, 9, 58);

	// sensor bus
	state.sensor_bus = dumbbus_new();
	init_jerk();
	// dumbbus_register(state.sensor_bus, &jerk);
	init_csdstate();
	// dumbbus_register(state.sensor_bus, &calib_stopdist);

	// time bus
	state.bus10hz = dumbbus_new();
	dumbbus_register(state.bus10hz, &ui_time);
	dumbbus_register(state.bus10hz, &printloc);

	state.simbus = dumbbus_new();
	dumbbus_register(state.simbus, &tick_engineer);

	state.eng = engineer_new(track == TRACK_A ? 'a' : 'b');
	state.cur_train = -1;
	state.last_sensor = NULL;

	calibrator_init();

	ui_init(&state);

	int tid_sensor_publisher = publisher_new(NULL, 9, sizeof(msg_sensor));
	publisher_sub(tid_sensor_publisher, MyTid());
	sensornotifier_new(tid_sensor_publisher);

	int tid_com2buffer = buffertask_new(NULL, 9, sizeof(msg_comin));
	comnotifier_new(tid_com2buffer, 9, COM2, state.con->tid_console);
	courier_new(9, tid_com2buffer, MyTid());

	int tid_refreshbuffer = buffertask_new(NULL, 9, sizeof(msg_time));
	timenotifier_new(tid_refreshbuffer, 9, MS2TICK(100));
	state.tid_refresh = courier_new(9, tid_refreshbuffer, MyTid());

	int tid_simstepbuffer = buffertask_new(NULL, 9, sizeof(msg_time));
	timenotifier_new(tid_simstepbuffer, 9, MS2TICK(15));
	state.tid_simstep = courier_new(9, tid_simstepbuffer, MyTid());

	void *msg = malloc(LEN_MSG);

	for (;;) {
		int tid;
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
				handle_time(&state, msg, tid);
				break;
			default:
				break;
		}
	}
}
