#include <train.h>
#include <syscall.h>
#include <uconst.h>
#include <string.h>
#include <stdio.h>
#include <console.h>
#include <lookup.h>
#include <fixed.h>
#include <dumbbus.h>
#include <server/sensorserver.h>
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

static void ui_sensor(a0state *state, char module, int id, int senstate) {
	if (senstate == OFF) return;
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

static void ui_reverse(a0state *state, int train, int t) {
	logdisplay_printf(state->log, "[%7d] reversed train %d", t, train);
	logdisplay_flushline(state->log);
}

static void ui_switch(a0state *state, char no, char pos, int t) {
	track_template_updateswitch(state->template, no, pos);
	console_flush(state->con);
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

static track ask_track(a0state *state) {
	for (;;) {
		console_clear(state->con);
		console_move(state->con, 1, 1);
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
#define STOP_PAUSE MS2TICK(2000)

struct {
	int state;
	int speed;
} csdstate;

static void init_csdstate() {
	csdstate.state = STOP_INIT;
	csdstate.speed = 6;
}

static void calib_stopdist(void* s) {
	a0state *state = s;
	engineer *eng = state->eng;
	train *train = engineer_get_train(eng, 39);
	track_node *cur_sensor = state->cur_sensor;
	track_node *e8 = engineer_get_tracknode(eng, "E", 8);
	switch (csdstate.state) {
		case STOP_INIT:
			engineer_set_switch(eng, 11, 'c');
			engineer_set_speed(eng, train->no, csdstate.speed);
			csdstate.state = STOP_UP2SPEED1;
			break;
		case STOP_UP2SPEED1:
			if (cur_sensor == e8) {
				csdstate.state = STOP_UP2SPEED2;
			}
			break;
		case STOP_UP2SPEED2:
			if (cur_sensor == e8) {
				csdstate.state = STOP_STOPPING;
			}
			break;
		case STOP_STOPPING:
			if (cur_sensor == e8) {
				logdisplay_printf(
					state->log,
					"testing speed idx %d",
					train_get_speedidx(train)
				);
				logdisplay_flushline(state->log);
				engineer_set_speed(eng, train->no, 0);
				engineer_set_switch(eng, 11, 's');
				Delay(STOP_PAUSE, eng->tid_time);
				if (csdstate.speed == 14) {
					csdstate.speed = 6;
				} else {
					csdstate.speed += 1;
				}
				engineer_reverse(eng, train->no);
				engineer_set_speed(eng, train->no, 14);
				csdstate.state = STOP_REVERSING;
			}
			break;
		case STOP_REVERSING:
			if (strcmp(cur_sensor->name, "E7") == 0) {
				engineer_set_switch(eng, 11, 'c');
				engineer_reverse(eng, train->no);
				engineer_set_speed(eng, train->no, csdstate.speed);
				csdstate.state = STOP_UP2SPEED1;
			}
			break;
	}
}

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

static void jerk(a0state *state, track_node *sensor, int timestamp) {
	engineer *eng = state->eng;
	track_node *last_sensor = j.last_sensor;
	int dx = track_distance(last_sensor, sensor);
	int dt = TICK2MS(timestamp - j.last_timestamp);
	j.last_sensor = sensor;
	j.last_timestamp = timestamp;

	switch (j.state) {
		case 0: // get to v_i
			logstrip_printf(state->cmdlog, "getting to v_i");
			engineer_set_speed(eng, 38, 8);
			j.state = 1;
			j.count = 0;
			break;
		case 1: // stabilize v_i
			logstrip_printf(state->cmdlog, "stabilizing v_i");
			j.count++;
			if (j.count == JERK_STABILIZE_NUM_SENSORS) {
				j.state = 2;
			}
			break;
		case 2: // get to v_f
			logstrip_printf(state->cmdlog, "getting to v_f");
			engineer_set_speed(eng, 38, 12);
			j.state = 3;
			j.count = 0;
			j.dx = 0;
			j.dt = 0;
			j.v_i_x = dx;
			j.v_i_t = dt;
			j.start = sensor;
			break;
		case 3: // stabilize v_f
			logstrip_printf(state->cmdlog, "stabilizing v_f");
			j.dx += dx;
			j.dt += dt;
			j.count++;
			if (j.count == JERK_STABILIZE_NUM_SENSORS) {
				// "dx/dt = %d/%d from %s to %s, v_i = %d/%d, v_f = %d/%d"
				logdisplay_printf(
					state->log,
					"%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d",
					j.start->name, sensor->name,
					j.dx, j.dt,
					j.v_i_x, j.v_i_t,
					dx, dt
				);
				logdisplay_flushline(state->log);
				j.state = 0;
			}
			break;
	}
}

struct {
	track_node *last_sensor;
	int t_last_sensor;
	int dx;
	int dt;
} app_v_avg_state;

static void init_v_avg() {
	app_v_avg_state.last_sensor = NULL;
	app_v_avg_state.dx = 0;
	app_v_avg_state.dt = 0;
}

static void get_v_avg(void* s) {
	a0state *state = s;
	engineer *eng = state->eng;
	train *train = engineer_get_train(eng, 39);

	track_node *last_sensor = app_v_avg_state.last_sensor;
	app_v_avg_state.last_sensor = state->cur_sensor;
	int t_last_sensor = app_v_avg_state.t_last_sensor;
	track_node *sensor = state->cur_sensor;
	int t_sensor = Time(state->tid_time);
	app_v_avg_state.t_last_sensor = t_sensor;

	if (!last_sensor) return;

	int dt = t_sensor - t_last_sensor;
	if (dt <= 0) return;

	int dx = track_distance(last_sensor, sensor);
	if (dx <= 0) return;

	app_v_avg_state.dx += dx;
	app_v_avg_state.dt += dt;

	logdisplay_printf(
		state->log,
		"[%7d] %s to %s is {%d} %d / %d (%F) [%d / %d (%F)]",
		t_sensor,
		last_sensor->name, sensor->name,
		train_get_speedidx(train),
		app_v_avg_state.dx, app_v_avg_state.dt,
		fixed_div(fixed_new(10000 * app_v_avg_state.dx / app_v_avg_state.dt), fixed_new(10000)),
		dx, dt,
		fixed_div(fixed_new(10000 * dx / dt), fixed_new(10000))
	);
	logdisplay_flushline(state->log);
}

static void handle_sensor(a0state *state, char rawmsg[]) {
	engineer *eng = state->eng;
	msg_sensor *m = (msg_sensor*) rawmsg;

	engineer_onsensor(eng, rawmsg);
	ui_sensor(state, m->module[0], m->id, m->state);

	// logdisplay_printf(
	// 	state->log,
	// 	"[%7d] %s%d %s",
	// 	m->timestamp, m->module, m->id, m->state == ON ? "on" : "off"
	// );
	// logdisplay_flushline(state->log);

	if (m->state == OFF) return;
	track_node *sensor = engineer_get_tracknode(eng, m->module, m->id);

	state->last_sensor = state->cur_sensor;
	state->cur_sensor = sensor;

	// jerk(state, sensor, m->timestamp); (THIS CODE HANGS THE CODE)
	dumbbus_dispatch(state->sensor_bus, state);
}

static void printstuff(engineer *eng, int train_no, logstrip *log1, logstrip *log2) {
	train *train = engineer_get_train(eng, train_no);

	location loc = train_get_frontloc(train);

	// print
	char *direction_str;
	switch (train_get_dir(train)) {
		case TRAIN_FORWARD:
			direction_str = ">";
			break;
		case TRAIN_BACKWARD:
			direction_str = "<";
			break;
		default:
			direction_str = "?";
			break;
	}

	location dest = train->destination;

	logstrip_printf(log1,
		"train %d at %L heading %s at %dmm/s (%dmm/s^2 -> %dmm/s^2) to %L",
		train_no,
		&loc,
		direction_str,
		fixed_int(fixed_mul(train_get_velocity(train), fixed_new(1000))),
		fixed_int(fixed_mul(train->a_i10k, fixed_new(10))),
		fixed_int(fixed_mul(train->a10k, fixed_new(10))),
		&dest
	);

	char msg[512];
	char *b = msg;
	b += sprintf(b, "\treserving:");
	for (int i = 0; i < train->reservation->len; i++) {
		location L = location_fromedge(train->reservation->edges[i]);
		b += sprintf(b, " %L", &L);
	}
	logstrip_printf(log2, msg);
}

static void printloc(void* s) {
	a0state *state = s;
	engineer *eng = state->eng;
	printstuff(eng, 37, state->trainloc1, state->trainloc1r);
	printstuff(eng, 38, state->trainloc2, state->trainloc2r);
}

static void handle_setup_demotrack(a0state *state) {
	engineer *eng = state->eng;
	int s[] = {6, 9, 10, 14, 15, 16,};
	int c[] = {1, 2, 3, 4, 5, 7, 8, 11, 12, 13, 17, 18, 153, 154, 155, 156};
	int ns = sizeof(s) / sizeof(int);
	int nc = sizeof(c) / sizeof(int);
	engineer_set_track(eng, s, ns, c, nc);
}

static void handle_train_switch_all(a0state *state, char pos) {
	engineer *eng = state->eng;
	int all[] = {
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		17, 18, 153, 154, 155, 156
	};
	int count = sizeof(all) / sizeof(int);
	if (track_switchpos_straight(pos)) {
		engineer_set_track(eng, all, count, NULL, 0);
	} else {
		engineer_set_track(eng, NULL, 0, all, count);
	}
}

static void handle_set_dest(a0state *state, int train_no, char *type, int id, int dist_cm) {
	train *train = engineer_get_train(state->eng, train_no);
	track_node *destnode = engineer_get_tracknode(state->eng, type, id);
	location dest =	location_fromnode(destnode, 0);
	location_add(&dest, fixed_new(dist_cm * 10));
	train_set_dest(train, &dest);
	char destname[100];
	location_tostring(&dest, destname);
	logstrip_printf(state->cmdlog, "Set train %d to go to %s", train_no, destname);
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

#define ENFORCE(test, msg) { \
	if (!(test)) { \
		errmsg = msg; \
		goto badcmd; \
	} \
}

static void handle_command(void* s, char *cmd, int size) {
	a0state *state = s;
	engineer *eng = state->eng;
	int quit = FALSE;
	char *errmsg = NULL;
	char *c = cmd;
	cmdline_clear(state->cmdline);

	if (!size) goto badcmd;

	switch (*c++) {
		case 'v':
			init_v_avg();
			break;
		case 'a': {
			ACCEPT('d');
			ACCEPT('d');
			ACCEPT(' ');
			int train = strgetui(&c);
			ENFORCE(train_goodtrain(train), "bad train");
			ACCEPT(' ');
			char sig1mod = *c++;
			ENFORCE(sig1mod >= 'A' && sig1mod <= 'E', "bad start sensor mod");
			int sig1id = strgetui(&c);
			ENFORCE(sig1id >= 1 && sig1id <= 16, "bad start sensor id");
			ACCEPT('\0');
			calibrate_train(state, train, sig1mod, sig1id);
			break;
		}
		case 'd': {
			ACCEPT('\0');
			handle_setup_demotrack(state);
			break;
		}
		case 'g': {
			if (*c == 'o') { // goto position (go #+ [a-zA-Z]+[0-9]+ (#+))
				ACCEPT('o');
				ACCEPT(' ');
				int train = strgetui(&c);
				if (!train_goodtrain(train)) goto badcmd;
				ACCEPT(' ');
				char type[4];
				uint len = strgetw(c, type, 4);
				if (len == 0) goto badcmd;
				c += len;
				int id = strgetui(&c);
				int dist_cm;
				if (*c == ' ') {
					ACCEPT(' ');
					dist_cm = strgetui(&c);
				} else {
					dist_cm = 0;
				}
				ACCEPT('\0');
				handle_set_dest(state, train, type, id, dist_cm);
			} else {
				goto badcmd;
			}
			break;
		}
		case 't': { // set train speed(tr # #)
			ACCEPT('r');
			ACCEPT(' ');
			int train = strgetui(&c);
			ENFORCE(train_goodtrain(train), "bad train");
			ACCEPT(' ');
			int speed = strgetui(&c);
			if (!train_goodspeed(speed)) goto badcmd;
			ACCEPT('\0');
			engineer_set_speed(eng, train, speed);
			break;
		}
		case 'r': { // reverse train (rv #+)
			ACCEPT('v');
			ACCEPT(' ');
			int train = strgetui(&c);
			ENFORCE(train_goodtrain(train), "bad train");
			ACCEPT('\0');
			engineer_reverse(eng, train);
			break;
		}
		case 'l': { // lose train (lt #+)
			ACCEPT('t');
			ACCEPT(' ');
			int train_no = strgetui(&c);
			ENFORCE(train_goodtrain(train_no), "bad train");
			ACCEPT('\0');
			train_set_lost(engineer_get_train(eng, train_no));
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
					engineer_set_switch(eng, id, pos);
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
	if (errmsg) {
		logstrip_printf(state->cmdlog, "bad command: \"%s\", %s", cmd, errmsg);
	} else {
		logstrip_printf(state->cmdlog, "bad command: \"%s\"", cmd);
	}

}

static void handle_comin(a0state *state, char msg[]) {
	msg_comin *comin = (msg_comin*) msg;
	ASSERT(comin->channel == COM2, "no handler for channel %d", comin->channel);
	if (comin->c == '=') {
		logdisplay_printf(state->log, "[%7d]", Time(state->tid_time));
		logdisplay_flushline(state->log);
		return;
	}
	cmdline_handleinput(state->cmdline, comin->c);
}

static void handle_time(a0state *state, char msg[], int tid) {
	msg_time *time = (msg_time*) msg;
	state->timestamp = time->timestamp;
	if (tid == state->tid_refresh) {
		dumbbus_dispatch(state->bus10hz, state);
	} else if (tid == state->tid_simstep) {
		dumbbus_dispatch(state->simbus, state);
	} else if (tid == state->tid_printloc) {
		dumbbus_dispatch(state->printlocbus, state);
	} else {
		ASSERT(0, "time message from unknown task");
	}
}

static void handle_traincmdmsgreceipt(a0state *state, char msg[]) {
	engineer *eng = state->eng;
	traincmd_receipt *rcpt = (traincmd_receipt*) msg;
	traincmd *cmd = &rcpt->cmd;
	int t = rcpt->timestamp;
		switch (cmd->name) {
			case SPEED: {
				int train_no = cmd->arg1;
				int speed = cmd->arg2;
				logdisplay_printf(state->log, "[%7d] set speed of train %d to %d, lag %dms", t, train_no, speed, t - cmd->timestamp);
				logdisplay_flushline(state->log);
				engineer_on_set_speed(eng, train_no, speed, t);
				break;
			}
			case REVERSE:
			case REVERSE_UI: {
				int train_no = cmd->arg1;
				ui_reverse(state, train_no, t);
				engineer_on_reverse(eng, train_no, t);
				break;
			}
			case SWITCH: {
				char no = cmd->arg1;
				char pos = cmd->arg2;
				engineer_on_set_switch(eng, no, pos, t);
				ui_switch(state, no, pos, t);
				logdisplay_printf(state->log, "[%7d] switched switch %d to %c, lag %dms", t, no, pos, t - cmd->timestamp);
				logdisplay_flushline(state->log);
				break;
			}
			case SOLENOID: {
				// logstrip_printf(state->cmdlog, "[%7d] cmdrcpt: offsolenoid()", t);
				break;
			}
			case QUERY1: {
				// logstrip_printf(state->cmdlog, "[%7d] cmdrcpt: querymod1(%d)", t, cmd->arg1);
				break;
			}
			case QUERY: {
				// logstrip_printf(state->cmdlog, "[%7d] cmdrcpt: querymods(%d)", t, cmd->arg1);
				break;
			}
			case GO: {
				// logstrip_printf(state->cmdlog, "[%7d] cmdrcpt: go()", t);
				break;
			}
			case STOP: {
				// logstrip_printf(state->cmdlog, "[%7d] cmdrcpt: stop()", t);
				break;
			}
			default:
				ERROR("bad train cmd: %d", cmd->name);
				break;
		}
}

void a0() {
	int mytid = MyTid();

	a0state state;
	state.tid_time = WhoIs(NAME_TIMESERVER);

	int tid_traincmdpub = publisher_new(NAME_TRAINCMDPUB, PRIORITY_TRAINCMDPUB, sizeof(traincmd_receipt));
	publisher_sub(tid_traincmdpub, mytid);

	// ui
	state.con = console_new(COM2);
	track track = ask_track(&state);
	state.template = track_template_new(state.con, track);
	state.cmdlog = logstrip_new(state.con, CONSOLE_LOG_LINE, CONSOLE_LOG_COL);
	state.cmdline = cmdline_new(state.con, CONSOLE_CMD_LINE, CONSOLE_CMD_COL, handle_command, &state);
	state.sensorlog = logstrip_new(state.con, CONSOLE_SENSOR_LINE, CONSOLE_SENSOR_COL);
	state.log = logdisplay_new(state.con, CONSOLE_DUMP_LINE, CONSOLE_DUMP_COL, 19, 55, ROUNDROBIN, "log");
	state.timedisplay = timedisplay_new(state.con, 1, 9);
	state.trainloc1 = logstrip_new(state.con, 2, 56 + 2);
	state.trainloc1r = logstrip_new(state.con, 3, 56 + 2);
	state.trainloc2 = logstrip_new(state.con, 4, 56 + 2);
	state.trainloc2r = logstrip_new(state.con, 5, 56 + 2);

	// sensor bus
	state.sensor_bus = dumbbus_new();
	// init_jerk();
	// dumbbus_register(state.sensor_bus, &jerk);
	// init_csdstate();
	// dumbbus_register(state.sensor_bus, &calib_stopdist);
	// init_v_avg();
	// dumbbus_register(state.sensor_bus, &get_v_avg);

	// time bus
	state.bus10hz = dumbbus_new();
	dumbbus_register(state.bus10hz, &ui_time);
	state.printlocbus = dumbbus_new();
	dumbbus_register(state.printlocbus, &printloc);

	state.simbus = dumbbus_new();
	dumbbus_register(state.simbus, &tick_engineer);

	state.eng = engineer_new(track == TRACK_A ? 'a' : 'b');

	state.last_sensor = NULL;

	calibrator_init();

//	int tid_uibuffer = buffertask_new(NULL, PRIORITY_UISERVER, 512); // TODO hardcorded item size
//	int tid_uiserver = Create(PRIORITY_UISERVER, uiserver);
//	courier_new(PRIORITY_UISERVER, tid_uibuffer, tid_uiserver);

	ui_init(&state);

	sensorserver_new();
	publisher_sub(WhoIs(NAME_SENSORPUB), mytid);

	int tid_com2buffer = buffertask_new(NULL, 9, sizeof(msg_comin));
	comnotifier_new(tid_com2buffer, 9, COM2, state.con->tid_console);
	courier_new(9, tid_com2buffer, mytid, sizeof(msg_comin), NULL);

	int tid_refreshbuffer = buffertask_new(NULL, 9, sizeof(msg_time));
	timenotifier_new(tid_refreshbuffer, 9, MS2TICK(100));
	state.tid_refresh = courier_new(9, tid_refreshbuffer, mytid, sizeof(msg_time), NULL);

	int tid_printlocbuffer = buffertask_new(NULL, 9, sizeof(msg_time));
	timenotifier_new(tid_printlocbuffer, 9, MS2TICK(200));
	state.tid_printloc = courier_new(9, tid_printlocbuffer, mytid, sizeof(msg_time), NULL);

	int tid_simstepbuffer = buffertask_new(NULL, 9, sizeof(msg_time));
	timenotifier_new(tid_simstepbuffer, 9, MS2TICK(20));
	state.tid_simstep = courier_new(9, tid_simstepbuffer, mytid, sizeof(msg_time), NULL);

	void *msg = malloc(LEN_MSG);

	handle_setup_demotrack(&state);

	for (;;) {
		int tid;
		int rcvlen = Receive(&tid, msg, LEN_MSG);
		Reply(tid, NULL, 0);
		ASSERT(rcvlen >= sizeof(msg_header), "bad data");
		msg_header *header = (msg_header*) msg;
		switch (header->type) {
			case MSG_SENSOR:
				handle_sensor(&state, msg);
				break;
			case MSG_COM_IN:
				handle_comin(&state, msg);
				break;
			case MSG_TIME:
				handle_time(&state, msg, tid);
				break;
			case MSG_TRAINCMDRECEIPT:
				handle_traincmdmsgreceipt(&state, msg);
				break;
			default:
				logdisplay_printf(
					state.log,
					"unhandled message %d",
					header->type
				);
				logdisplay_flushline(state.log);
				break;
		}
	}
}
