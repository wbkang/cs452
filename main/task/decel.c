#include <task/decel.h>
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

#define LEN_MSG (64 * 4)
#define TRAINNO 35
#define CRUISING_SPEED 12
#define MAX_DIST 700
#define OFFSET_INTERVAL 100
#define ACCELERATION_TIME MS2TICK(10000)

static engineer* eng;
static a0ui* decel_ui;
static location reference_location;
static location stop_location;
static location actual_stop_location;
static int last_stop_time;
static int last_start_time;
static int stop_offset;
static enum {
	DECEL_ACCEL,
	DECEL_STOPPING,
	DECEL_IDLE
} decel_state;

static void handle_setup_demotrack() {
	int s[] = {/*6,*/ 9, 10, 14, 15, 16};
	int c[] = {1, 2, 3, 4, 5, 7, 8, 11, 12, 13, 17, /*18,*/ 153, 154, 155, 156};
	int ns = sizeof(s) / sizeof(int);
	int nc = sizeof(c) / sizeof(int);
	engineer_set_track(eng, s, ns, c, nc);
}

static void handle_command(void* vthis, char *cmd, int size) {
	a0ui_on_cmdreset(decel_ui);

	int dist = strgetui(&cmd);
//
//	start_loc = reference_location;
//	location_add(&start_loc, dist * -10);
//		char buf[1024];
//		location_tostring(&start_loc);
//		a0ui_on_log(decel_ui, buf);

	if (decel_state == DECEL_IDLE) {
		train* train = engineer_get_train(eng, TRAINNO);
		train_set_dir(train, TRAIN_FORWARD);
		track_node *start = engineer_get_tracknode(eng, "B", 16);
		location startloc = location_fromnode(start, 0);
		train_set_frontloc(train, &startloc);
	}

	if (decel_state == DECEL_IDLE || decel_state == DECEL_STOPPING) {
		stop_offset = 0;
		stop_location = reference_location;
		location_add(&stop_location, dist * 10);
		engineer_set_speed(eng, TRAINNO, CRUISING_SPEED);
	}

}

static void decel_on_sensor(msg_sensor *msg) {
	if (!msg->state) return;

	engineer_onsensor(eng, msg);
	a0ui_on_sensor(decel_ui, msg->module[0], msg->id, msg->state);

	if (decel_state == DECEL_STOPPING) {
		track_node *sensor = engineer_get_tracknode(eng, msg->module, msg->id);
		location sensorloc = location_fromnode(sensor, 0);

		int timediff = msg->timestamp - last_stop_time;
		int dist = location_dist_dir(&actual_stop_location, &sensorloc);

		a0ui_on_logf(decel_ui, "%d,%d,", dist, timediff);
	}
}

static void decel_on_tick(int time) {
	if (decel_state == DECEL_ACCEL) {
		if (time >= last_start_time + ACCELERATION_TIME) {
			train *train = engineer_get_train(eng, TRAINNO);
			location curloc = train_get_pickuploc(train);
			int dist = location_dist_min(&curloc, &stop_location);
			if (0 <= dist && dist < 10) {
				engineer_set_speed(eng, TRAINNO, 0);
			}
		}
	}
}

static void decel_on_traincmdmsgreceipt(traincmd_receipt* msg) {
	traincmd *cmd = &msg->cmd;
	int t = msg->timestamp;
		switch (cmd->name) {
			case SPEED: {
				int train_no = cmd->arg1;
				int speed = cmd->arg2;
//				a0ui_on_logf(decel_ui, "[%7d] set speed of train %d to %d, lag %dms", t, train_no, speed, t - cmd->timestamp);
				engineer_on_set_speed(eng, train_no, speed, t);
				if (speed == 0) {
					last_stop_time = msg->timestamp;
					decel_state = DECEL_STOPPING;
					train *train = engineer_get_train(eng, train_no);
					actual_stop_location = train_get_pickuploc(train);
				} else {
					last_start_time = msg->timestamp;
					decel_state = DECEL_ACCEL;
				}
				break;
			}
			case REVERSE:
			case REVERSE_UI: {
				int train_no = cmd->arg1;
				a0ui_on_reverse(decel_ui, train_no, t);
				engineer_on_reverse(eng, train_no, t);
				break;
			}
			case SWITCH: {
				char no = cmd->arg1;
				char pos = cmd->arg2;
				engineer_on_set_switch(eng, no, pos, t);
				a0ui_on_switch(decel_ui, no, pos, t);
//				a0ui_on_logf(decel_ui, "[%7d] switched switch %d to %c, lag %dms", t, no, pos, t - cmd->timestamp);
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



void decel() {
	int mytid = MyTid();
	// int tid_time = WhoIs(NAME_TIMESERVER);
	int tid_traincmdpub = publisher_new(NAME_TRAINCMDPUB, PRIORITY_TRAINCMDPUB, sizeof(traincmd_receipt));
	publisher_sub(tid_traincmdpub, mytid);

	// ui
	uiserver_new();
	/*ui_id id_ui = */uiserver_register();
	char track = 'b';
	decel_ui = a0ui_new(handle_command, track);

	eng = engineer_new(track, decel_ui);

	decel_state = DECEL_IDLE;

	track_node *refnode = engineer_get_tracknode(eng, "C", 5);
	reference_location = location_fromnode(refnode, 0);

	calibrator_init();
	sensorserver_new();
	publisher_sub(WhoIs(NAME_SENSORPUB), mytid);

	int tid_com2buffer = buffertask_new(NULL, 9, sizeof(msg_comin));
	comnotifier_new(tid_com2buffer, 9, COM2, WhoIs(NAME_IOSERVER_COM2));
	courier_new(9, tid_com2buffer, mytid, sizeof(msg_comin), NULL);

	int tid_simstepbuffer = buffertask_new(NULL, 9, sizeof(msg_time));
	timenotifier_new(tid_simstepbuffer, 9, MS2TICK(20));
	/*int tid_simstep = */courier_new(9, tid_simstepbuffer, mytid, sizeof(msg_time), NULL);

	void *msg = malloc(LEN_MSG);

	handle_setup_demotrack();

	for (;;) {
		int tid;
		int rcvlen = Receive(&tid, msg, LEN_MSG);
		ASSERT(rcvlen >= sizeof(msg_header), "bad data rcvlen:%d from %d", rcvlen, tid);
		int rplylen = Reply(tid, NULL, 0);
		ASSERT(rplylen >= 0, "reply failed to %d", tid);
		msg_header *header = (msg_header*) msg;

		switch (header->type) {
			case MSG_SENSOR:
				decel_on_sensor((msg_sensor*)msg);
				break;
			case MSG_COM_IN:
				a0ui_on_key_input(decel_ui, ((msg_comin*)msg)->c, NULL);
				break;
			case MSG_TRAINCMDRECEIPT:
				decel_on_traincmdmsgreceipt((traincmd_receipt*)msg);
				break;
			case MSG_TIME:
				engineer_ontick(eng, NULL);
				decel_on_tick(((msg_time*)msg)->timestamp);
				break;
			default:
				ASSERT(0, "unhandled message %d from %d", header->type, tid);
				break;
		}
	}
}
