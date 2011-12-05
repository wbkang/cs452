#include <task/accel.h>
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
#define TRAINNO 39
#define CRUISING_SPEED 6
#define SENSOR_TRIAL 5

static engineer* eng;
static a0ui* accel_ui;
static location reference_location;
static location start_loc;
static int sensor_count;
static int start_time;

static int times[10];
static int dists[10];

static void handle_setup_demotrack() {
	int s[] = {/*6,*/ 9, 10, 14, 15, 16};
	int c[] = {1, 2, 3, 4, 5, 7, 8, 11, 12, 13, 17, /*18,*/ 153, 154, 155, 156};
	int ns = sizeof(s) / sizeof(int);
	int nc = sizeof(c) / sizeof(int);
	engineer_set_track(eng, s, ns, c, nc);
}

static void handle_command(void* vthis, char *cmd, int size) {
	a0ui_on_cmdreset(accel_ui);

	int dist = strgetui(&cmd);
	sensor_count = 0;

	start_loc = reference_location;
	location_add(&start_loc, dist * -10);
//		char buf[1024];
//		location_tostring(&start_loc);
//		a0ui_on_log(accel_ui, buf);
	engineer_set_speed(eng, TRAINNO, CRUISING_SPEED);

}

static void accel_on_sensor(msg_sensor *msg) {
	if (!msg->state) return;
	sensor_count++;

	engineer_onsensor(eng, msg);
	a0ui_on_sensor(accel_ui, msg->module[0], msg->id, msg->state);

	track_node *sensor = engineer_get_tracknode(eng, msg->module, msg->id);
	location sensorloc = location_fromnode(sensor, 0);

	int timediff = msg->timestamp - start_time;
	int dist = location_dist_dir(&start_loc, &sensorloc);


	if (sensor_count == SENSOR_TRIAL) {
		engineer_set_speed(eng, TRAINNO, 0);
		char buf[1024];
		for (int i = 0; i < SENSOR_TRIAL - 1; i++) {
			sprintf(buf, "%d,%d,", dists[i], times[i]);
			a0ui_on_log(accel_ui, buf);
		}
	} else {
		times[sensor_count - 1] = timediff;
		dists[sensor_count - 1] = dist;
	}
}

static void accel_on_traincmdmsgreceipt(traincmd_receipt* msg) {
	traincmd *cmd = &msg->cmd;
	int t = msg->timestamp;
		switch (cmd->name) {
			case SPEED: {
				int train_no = cmd->arg1;
				int speed = cmd->arg2;
//				a0ui_on_logf(accel_ui, "[%7d] set speed of train %d to %d, lag %dms", t, train_no, speed, t - cmd->timestamp);
				engineer_on_set_speed(eng, train_no, speed, t);
				start_time = msg->timestamp;
				break;
			}
			case REVERSE:
			case REVERSE_UI: {
				int train_no = cmd->arg1;
				a0ui_on_reverse(accel_ui, train_no, t);
				engineer_on_reverse(eng, train_no, t);
				break;
			}
			case SWITCH: {
				char no = cmd->arg1;
				char pos = cmd->arg2;
				engineer_on_set_switch(eng, no, pos, t);
				a0ui_on_switch(accel_ui, no, pos, t);
//				a0ui_on_logf(accel_ui, "[%7d] switched switch %d to %c, lag %dms", t, no, pos, t - cmd->timestamp);
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



void accel() {
	int mytid = MyTid();
	WhoIs(NAME_TIMESERVER);
	int tid_traincmdpub = publisher_new(NAME_TRAINCMDPUB, PRIORITY_TRAINCMDPUB, sizeof(traincmd_receipt));
	publisher_sub(tid_traincmdpub, mytid);

	// ui
	uiserver_new();
	char track = 'b';
	accel_ui = a0ui_new(handle_command, track);

	eng = engineer_new(track, accel_ui);

	track_node *refnode = engineer_get_tracknode(eng, "A", 1);
	reference_location = location_fromnode(refnode, 0);
	start_loc = reference_location;

	sensorserver_new();
	publisher_sub(WhoIs(NAME_SENSORPUB), mytid);

	int tid_com2buffer = buffertask_new(NULL, 9, sizeof(msg_comin));
	comnotifier_new(tid_com2buffer, 9, COM2, WhoIs(NAME_IOSERVER_COM2));
	courier_new(9, tid_com2buffer, mytid, sizeof(msg_comin), NULL);

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
				accel_on_sensor((msg_sensor*)msg);
				break;
			case MSG_COM_IN:
				a0ui_on_key_input(accel_ui, ((msg_comin*)msg)->c, NULL);
				break;
			case MSG_TRAINCMDRECEIPT:
				accel_on_traincmdmsgreceipt((traincmd_receipt*)msg);
				break;
			default:
				ASSERT(0, "unhandled message %d from %d", header->type, tid);
				break;
		}
	}
}
