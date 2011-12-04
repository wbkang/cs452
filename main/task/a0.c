#include <train.h>
#include <syscall.h>
#include <uconst.h>
#include <string.h>
#include <stdio.h>
#include <console.h>
#include <lookup.h>
#include <dumbbus.h>
#include <server/sensorserver.h>
#include <server/comnotifier.h>
#include <server/timenotifier.h>
#include <task/a0.h>
#include <ui/a0ui.h>
#include <train_calibrator.h>
#include <train_stopper.h>
#include <server/buffertask.h>
#include <server/uiserver.h>
#include <server/courier.h>
#include <server/publisher.h>
#include <track_node.h>
#include <util.h>
#include <engineer.h>

a0state *state;

a0state *get_state() {
	return state;
}

#define LEN_MSG (64 * 4)


/*
 * Server
 */

static char ask_track() {
	a0state *state = get_state();
	uiserver_force_refresh(state->id_ui);
	for (;;) {
		uiserver_move(state->id_ui, 1, 1);
		uiserver_printf(state->id_ui, "%-100s", "Track a or b?\n");
		char c = Getc(COM2, WhoIs(NAME_IOSERVER_COM2));
		switch (c) {
			case 'a':
			case 'b':
				return c;
			default:
				uiserver_move(state->id_ui, 1, 1);
				uiserver_printf(state->id_ui, "%-100s", "fail");
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

static void calib_stopdist() {
	a0state *state = get_state();
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
				a0ui_on_logf(
					state->a0ui,
					"testing speed idx %d",
					train_get_speedidx(train)
				);
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

static void jerk(track_node *sensor, int timestamp) {
	a0state *state = get_state();
	engineer *eng = state->eng;
	track_node *last_sensor = j.last_sensor;
	int dx = track_distance(last_sensor, sensor);
	int dt = TICK2MS(timestamp - j.last_timestamp);
	j.last_sensor = sensor;
	j.last_timestamp = timestamp;

	switch (j.state) {
		case 0: // get to v_i
			a0ui_on_cmdlog(state->a0ui, "getting to v_i");
			engineer_set_speed(eng, 38, 8);
			j.state = 1;
			j.count = 0;
			break;
		case 1: // stabilize v_i
			a0ui_on_cmdlog(state->a0ui, "stabilizing v_i");
			j.count++;
			if (j.count == JERK_STABILIZE_NUM_SENSORS) {
				j.state = 2;
			}
			break;
		case 2: // get to v_f
			a0ui_on_cmdlog(state->a0ui, "getting to v_f");
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
			a0ui_on_cmdlog(state->a0ui, "stabilizing v_f");
			j.dx += dx;
			j.dt += dt;
			j.count++;
			if (j.count == JERK_STABILIZE_NUM_SENSORS) {
				// "dx/dt = %d/%d from %s to %s, v_i = %d/%d, v_f = %d/%d"
				a0ui_on_logf(
					state->a0ui,
					"%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d",
					j.start->name, sensor->name,
					j.dx, j.dt,
					j.v_i_x, j.v_i_t,
					dx, dt
				);
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

static void get_v_avg(void *vstate, void* unused) {
	(void) unused;
	a0state *state = vstate;
	engineer *eng = state->eng;
	train *train = engineer_get_train(eng, 21);

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

	a0ui_on_logf(
		state->a0ui,
		"[%7d] %s to %s is {%d} %d / %d (%f) [%d / %d (%f)]",
		t_sensor,
		last_sensor->name, sensor->name,
		train_get_speedidx(train),
		app_v_avg_state.dx, app_v_avg_state.dt,
		((float) app_v_avg_state.dx) / app_v_avg_state.dt,
		dx, dt,
		((float) dx) / dt
	);
}

static void handle_sensor(msg_sensor *msg) {
	a0state *state = get_state();

	engineer_onsensor(state->eng, msg);
	a0ui_on_sensor(state->a0ui, msg->module[0], msg->id, msg->state);

	// logdisplay_printf(
	// 	state->log,
	// 	"[%7d] %s%d %s",
	// 	msg->timestamp, msg->module, msg->id, msg->state == ON ? "on" : "off"
	// );
	// logdisplay_flushline(state->log);

	if (msg->state == OFF) return;
	track_node *sensor = engineer_get_tracknode(state->eng, msg->module, msg->id);

	state->last_sensor = state->cur_sensor;
	state->cur_sensor = sensor;

	// jerk(state, sensor, m->timestamp);
	dumbbus_dispatch(state->sensor_bus, state, NULL);
}

static void handle_setup_demotrack() {
	engineer *eng = get_state()->eng;
	int s[] = {/*6,*/ 9, 10, 14, 15, 16};
	int c[] = {1, 2, 3, 4, 5, 7, 8, 11, 12, 13, 17, /*18,*/ 153, 154, 155, 156};
	int ns = sizeof(s) / sizeof(int);
	int nc = sizeof(c) / sizeof(int);
	engineer_set_track(eng, s, ns, c, nc);
}

static void handle_train_switch_all(char pos) {
	engineer *eng = get_state()->eng;
	int all[] = {
		1, 2, 3, 4, 5, /*6,*/ 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		17, /*18,*/ 153, 154, 155, 156
	};
	int count = sizeof(all) / sizeof(int);
	if (track_switchpos_straight(pos)) {
		engineer_set_track(eng, all, count, NULL, 0);
	} else {
		engineer_set_track(eng, NULL, 0, all, count);
	}
}

static void handle_set_dest(int train_no, char *type, int id, int dist_cm) {
	a0state *state = get_state();
	train *train = engineer_get_train(state->eng, train_no);
	track_node *node_dest = engineer_get_tracknode(state->eng, type, id);
	location loc_dest =	location_fromnode(node_dest, 0);
	location_add(&loc_dest, dist_cm * 10);
	train_set_dest(train, &loc_dest);
	char name_dest[32];
	location_tostring(&loc_dest, name_dest);
	a0ui_on_cmdlogf(state->a0ui, "Set train %d to go to %s", train_no, name_dest);
}

// over must be in mm
static void handle_train_stopper_setup(int train_no, char *type, int id, int over) {
	a0state *state = get_state();
	engineer *eng = state->eng;
	ASSERTNOTNULL(type);
	ASSERT(over >= 0, "dist negative: %d", over);

	track_node *loc_dest = engineer_get_tracknode(eng, type, id);
	if (!loc_dest) {
		a0ui_on_cmdlogf(state->a0ui, "Sorry. Landmark %s%d does not exist.", type, id);
		return;
	}

	location dest = location_fromnode(loc_dest, 0);
	location_add(&dest, over);
	if (location_isundef(&dest)) {
		a0ui_on_cmdlogf(state->a0ui,
			"Sorry. Location %s+%dmm is invalid.",
			loc_dest->name,
			over
		);
		return;
	}

	train_stopper_setup(state->simbus, engineer_get_train(eng, train_no), &dest);
	a0ui_on_cmdlogf(state->a0ui,
		"working on stopping train %d at %s+%dmm",
		train_no,
		loc_dest->name,
		over
	);
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


static void handle_command(void* vthis, char *cmd, int size) {
	a0state *state = vthis;
	engineer *eng = state->eng;
	char *errmsg = NULL;
	char *c = cmd;
	a0ui_on_cmdreset(state->a0ui);

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
			calibrate_train(train, sig1mod, sig1id);
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
				ENFORCE(train_goodtrain(train), "bad train");
				ACCEPT(' ');
				char type[4];
				uint len = strgetw(c, type, 4);
				ENFORCE(len > 0, "bad location");
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
				handle_set_dest(train, type, id, dist_cm);
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
			ENFORCE(train_goodspeed(speed), "bad speed");
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
				ENFORCE(train_goodtrain(train), "bad train");
				ACCEPT(' ');
				char type[4];
				uint len = strgetw(c, type, 4);
				if (len == 0) goto badcmd;
				c += len;
				int id = strgetui(&c);
				ACCEPT(' ');
				int dist_cm = strgetui(&c);
				ACCEPT('\0');
				handle_train_stopper_setup(train, type, id, 10 * dist_cm);
			} else if (*c == 'w') { // set switch position (sw #+ [cCsS])
				ACCEPT('w');
				ACCEPT(' ');
				char id;
				if (*c == '*') {
					id = *c++;
				} else {
					id = strgetui(&c);
					ENFORCE(train_goodswitch(id), "bad switch");
				}
				ACCEPT(' ');
				char pos = *c++;
				ENFORCE(track_switchpos_isgood(pos), "bad switch position");
				ACCEPT('\0');
				if (id == '*') {
					handle_train_switch_all(pos);
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
			a0_destroy();
			break;
		}
		case 'z': {
			ACCEPT('\0');
			uiserver_force_refresh(state->id_ui);
			break;
		}
		default: {
			goto badcmd;
			break;
		}
	}

	return;

	badcmd:
	if (errmsg) {
		a0ui_on_cmdlogf(state->a0ui, "bad command: \"%s\", %s", cmd, errmsg);
	} else {
		a0ui_on_cmdlogf(state->a0ui, "bad command: \"%s\"", cmd);
	}

}

static void handle_comin(msg_comin *comin) {
	a0state *state = get_state();
	ASSERT(comin->channel == COM2, "no handler for channel %d", comin->channel);
	if (comin->c == '=') {
		a0ui_on_cmdlogf(state->a0ui, "[%7d]", Time(state->tid_time));
		return;
	}
	a0ui_on_key_input(state->a0ui, comin->c, state);
}

static void handle_time(msg_time *time, int tid) {
	a0state *state = get_state();
	if (tid == state->tid_refresh) {
		dumbbus_dispatch(state->bus10hz, state->a0ui, (void*)time->timestamp);
	} else if (tid == state->tid_simstep) {
		dumbbus_dispatch(state->simbus, state->eng, state);
	} else {
		ASSERT(0, "time message from unknown task");
	}
}

static void handle_traincmdmsgreceipt(traincmd_receipt *rcpt) {
	a0state *state = get_state();
	engineer *eng = state->eng;
	traincmd *cmd = &rcpt->cmd;
	int t = rcpt->timestamp;
		switch (cmd->name) {
			case SPEED: {
				int train_no = cmd->arg1;
				int speed = cmd->arg2;
				a0ui_on_logf(state->a0ui, "[%7d] set speed of train %d to %d, lag %dms", t, train_no, speed, t - cmd->timestamp);
				engineer_on_set_speed(eng, train_no, speed, t);
				break;
			}
			case REVERSE:
			case REVERSE_UI: {
				int train_no = cmd->arg1;
				a0ui_on_reverse(state->a0ui, train_no, t);
				engineer_on_reverse(eng, train_no, t);
				break;
			}
			case SWITCH: {
				char no = cmd->arg1;
				char pos = cmd->arg2;
				engineer_on_set_switch(eng, no, pos, t);
				a0ui_on_switch(state->a0ui, no, pos, t);
				a0ui_on_logf(state->a0ui, "[%7d] switched switch %d to %c, lag %dms", t, no, pos, t - cmd->timestamp);
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

void a0_destroy() {
	a0state *state = get_state();
	engineer_destroy(state->eng);
	a0ui_on_quit(state->a0ui);
	ExitKernel(0);
}

void a0() {
	int mytid = MyTid();
	state = malloc(sizeof(a0state));
	state->tid_time = WhoIs(NAME_TIMESERVER);

	int tid_traincmdpub = publisher_new(NAME_TRAINCMDPUB, PRIORITY_TRAINCMDPUB, sizeof(traincmd_receipt));
	publisher_sub(tid_traincmdpub, mytid);

	// ui
	uiserver_new();
	state->id_ui = uiserver_register();
	char track = ask_track(&state);
	state->a0ui = a0ui_new(handle_command, track);
	// sensor bus
	state->sensor_bus = dumbbus_new();
	// init_csdstate();
	// dumbbus_register(state->sensor_bus, &calib_stopdist);
	// init_v_avg();
	// dumbbus_register(state->sensor_bus, &get_v_avg);

	// time bus
	state->bus10hz = dumbbus_new();
	dumbbus_register(state->bus10hz, a0ui_on_time);
	state->simbus = dumbbus_new();
	dumbbus_register(state->simbus, engineer_ontick);

	state->eng = engineer_new(track, state->a0ui);
	state->last_sensor = NULL;

	calibrator_init();

	sensorserver_new();
	publisher_sub(WhoIs(NAME_SENSORPUB), mytid);

	int tid_com2buffer = buffertask_new(NULL, 9, sizeof(msg_comin));
	comnotifier_new(tid_com2buffer, 9, COM2, WhoIs(NAME_IOSERVER_COM2));
	courier_new(9, tid_com2buffer, mytid, sizeof(msg_comin), NULL);

	int tid_refreshbuffer = buffertask_new(NULL, 9, sizeof(msg_time));
	timenotifier_new(tid_refreshbuffer, 9, MS2TICK(100));
	state->tid_refresh = courier_new(9, tid_refreshbuffer, mytid, sizeof(msg_time), NULL);

	int tid_simstepbuffer = buffertask_new(NULL, 9, sizeof(msg_time));
	timenotifier_new(tid_simstepbuffer, 9, MS2TICK(20));
	state->tid_simstep = courier_new(9, tid_simstepbuffer, mytid, sizeof(msg_time), NULL);

	void *msg = malloc(LEN_MSG);

	handle_setup_demotrack(&state);

	for (;;) {
		int tid;
		int rcvlen = Receive(&tid, msg, LEN_MSG);
		ASSERT(rcvlen >= sizeof(msg_header), "bad data rcvlen:%d from %d", rcvlen, tid);
		int rplylen = Reply(tid, NULL, 0);
		ASSERT(rplylen >= 0, "reply failed to %d", tid);
		msg_header *header = (msg_header*) msg;

		switch (header->type) {
			case MSG_SENSOR:
				handle_sensor(msg);
				break;
			case MSG_COM_IN:
				handle_comin(msg);
				break;
			case MSG_TIME:
				handle_time(msg, tid);
				break;
			case MSG_TRAINCMDRECEIPT:
				handle_traincmdmsgreceipt(msg);
				break;
			default:
				ASSERT(0, "unhandled message %d from %d", header->type, tid);
				break;
		}
	}
}
