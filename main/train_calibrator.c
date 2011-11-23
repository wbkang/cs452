#include <task/a0.h>
#include <dumbbus.h>
#include <string.h>
#include <train_calibrator.h>
#include <gps.h>
#include <util.h>

#define CALIB_SWITCHDELAY MS2TICK(3000)
#define CALIB_SENSOR_START "E8"
#define CALIB_SENSOR_END "C14"
#define CALIB_SENSOR_WRONG_DIR "E7"

static struct {
	enum { IDLE, ORIENTING, GO2END, SKIPPING, CALIBRATING } state;
	int train_no;
	track_node *front;
	track_node *back;

	int min_speed;
	int max_speed;
	int ascending;
	int cur_speed;
	int testrun;
} calib_state;

static void handle_sensor_response(void* s);

void calibrator_init() {
	calib_state.state = IDLE;
}

static int next_calib_speed() {
	if (calib_state.ascending) {
		calib_state.cur_speed++;
		if (calib_state.cur_speed > calib_state.max_speed) {
			calib_state.cur_speed = calib_state.max_speed - 1;
			calib_state.ascending = 0;
		}
	} else {
		calib_state.cur_speed--;
	}

	if (calib_state.cur_speed >= calib_state.min_speed && calib_state.max_speed >= calib_state.cur_speed) {
		return calib_state.cur_speed;
	} else {
		return -1;
	}
}

static void calibrator_quit(a0state *state) {
	dumbbus_unregister(state->sensor_bus, &handle_sensor_response);
	calibrator_init();
	logstrip_printf(state->cmdlog, "finished calibrating train %d", calib_state.train_no);
}

static void calibrator_start(engineer *eng, int train_no) {
	engineer_train_set_dir(eng, train_no, TRAIN_FORWARD);
	engineer_set_speed(eng, train_no, 0);
	Delay(2000, eng->tid_time);
	engineer_set_speed(eng, train_no, calib_state.min_speed);
	calib_state.cur_speed = calib_state.min_speed;
	calib_state.state = CALIBRATING;
}

static void handle_sensor_response(void* s) {
	a0state *state = s;
	engineer *eng = state->eng;

	int train_no = calib_state.train_no;

	track_node *sensor = state->cur_sensor;
	ASSERTNOTNULL(sensor);
	track_node *last_sensor = state->last_sensor;

	switch (calib_state.state) {
		case IDLE:
			ASSERT(0, "shouldn't be idle");
			break;
		case ORIENTING:
			if (sensor == calib_state.front) { // correct
				engineer_train_set_dir(eng, train_no, TRAIN_FORWARD);
				engineer_set_speed(eng, train_no, 0);
				calibrator_quit(state);
				state->cur_train = train_no;
				// calibrator_start(eng, train_no);
			} else if (sensor == calib_state.back) {
				engineer_train_set_dir(eng, train_no, TRAIN_BACKWARD);
				engineer_reverse(eng, train_no);
				engineer_set_speed(eng, train_no, 0);
				calibrator_quit(state);
				state->cur_train = train_no;
				// calib_state.state = GO2END;
			} else {
				engineer_set_speed(eng, train_no, 0);
				calibrator_quit(state);
				logstrip_printf(state->cmdlog,
						"train %d in bad position, exiting calibration. (expected: %s or %s, given: %s)",
						train_no, CALIB_SENSOR_END, CALIB_SENSOR_WRONG_DIR, sensor->name);
			}
			break;
		case GO2END:
			if (strcmp(sensor->name, CALIB_SENSOR_END) == 0) {
				calibrator_start(eng, train_no);
			}
			break;
		case CALIBRATING:
			if (strcmp(sensor->name, CALIB_SENSOR_END) == 0) {
				if (strcmp(last_sensor->name, CALIB_SENSOR_START) != 0) {
					logstrip_printf(state->cmdlog, "spurious sensor %s, ignoring this run", sensor->name);
					break;
				}
				// set up the next calibration
				int speed = next_calib_speed();
				if (speed < 0) {
					engineer_set_speed(eng, train_no, 0);
					logstrip_printf(state->cmdlog, "calibration: finished. current train set to %d", train_no);
					calibrator_quit(state);
					state->cur_train = train_no;
				} else {
					engineer_set_speed(eng, train_no, speed);
					logstrip_printf(state->cmdlog, "next calibration: speed %d", speed);
				}
			}
			break;
		default:
			ASSERT(0, "bad state");
			break;
	}
}

static void calibrator_setup_track(a0state *state) {
	engineer *eng = state->eng;
	int s[] = {1, 2, 4, 6, 7, 9, 14, 16, 153, 155};
	int c[] = {3, 5, 8, 10, 11, 12, 13, 15, 17, 18, 154, 156};
	int ns = sizeof(s) / sizeof(int);
	int nc = sizeof(c) / sizeof(int);
	engineer_set_track(eng, s, ns, c, nc);
}

void calibrate_train(a0state *state, int train_no, char sig1mod, int sig1id, char sig2mod, int sig2id) {
	engineer *eng = state->eng;

	if (calib_state.state != IDLE) {
		calibrator_quit(state);
	}

	char mod[2];
	mod[1] = '\0';

	mod[0] = sig1mod;
	track_node *front = engineer_get_tracknode(eng, mod, sig1id);
	if (!front) {
		logstrip_printf(state->cmdlog, "bad front sensor %c%d", sig1mod, sig1id);
		return;
	}

	mod[0] = sig2mod;
	track_node *back = engineer_get_tracknode(eng, mod, sig2id);
	if (!back) {
		logstrip_printf(state->cmdlog, "bad back sensor %c%d", sig2mod, sig2id);
		return;
	}

	// stop the train
	engineer_set_speed(eng, train_no, 0);

	// set track to calibration mode
	// calibrator_setup_track(state);
	// Delay(CALIB_SWITCHDELAY, eng->tid_time);

	// set currently active train
	calib_state.train_no = train_no;

	// initialize
	calib_state.state = ORIENTING;
	engineer_set_speed(eng, train_no, CALIB_ORIENTING_SPEED);
	calib_state.front = front;
	calib_state.back = back;

	logstrip_printf(state->cmdlog, "orienting train %d", train_no);

	// register for sensor notifications
	dumbbus_register(state->sensor_bus, &handle_sensor_response);
}
