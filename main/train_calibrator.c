#include <task/a0.h>
#include <dumbbus.h>
#include <string.h>
#include <train_calibrator.h>

#define CALIB_SENSOR_START "E8"
#define CALIB_SENSOR_END "C14"
#define CALIB_SENSOR_WRONG_DIR "E7"

static struct {
	enum { IDLE, ORIENTING, GO2END, SKIPPING, CALIBRATING } state;
	int min_speed;
	int max_speed;
	int ascending;
	int cur_speed;
	int train_no;
	int testrun;
} calib_state;

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

static void calibrator_start(engineer *eng, int train_no) {
	engineer_set_speed(eng, train_no, calib_state.min_speed - 1);
	engineer_set_speed(eng, train_no, calib_state.min_speed);
	calib_state.cur_speed = calib_state.min_speed;
	calib_state.state = CALIBRATING;
}

static void handle_sensor_response(void* s) {
	a0state *state = s;
	engineer *eng = state->eng;

	int train_no = calib_state.train_no;

	track_node *sensor = state->cur_node;
	ASSERTNOTNULL(sensor);
	track_node *last_sensor = state->last_node;

	switch (calib_state.state) {
		case IDLE:
			ASSERT(0, "shouldn't be idle");
			break;
		case ORIENTING:
			if (strcmp(sensor->name, CALIB_SENSOR_END) == 0) { // correct
				calibrator_start(eng, train_no);
			} else if (strcmp(sensor->name, CALIB_SENSOR_WRONG_DIR) == 0) {
				engineer_reverse(eng, train_no);
				calib_state.state = GO2END;
			} else {
				engineer_set_speed(eng, train_no, 0);
				calibrator_init();
				logdisplay_printf(state->expected_time_display, "train %d is not in the correct position, exiting calibration.", train_no);
				logdisplay_flushline(state->expected_time_display);
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
					logdisplay_printf(state->expected_time_display, "spurious sensor %s, ignoring this run", sensor->name);
					logdisplay_flushline(state->expected_time_display);
					break;
				}
				int tref = state->cur_tick - state->last_tick;
				int speed = calib_state.cur_speed;
				int speed_idx = engineer_get_speedidx(eng, train_no);
				engineer_set_tref(eng, train_no, speed_idx, tref);
				logdisplay_printf(state->expected_time_display, "tref set for train %d, speed %d, to %d. speed_idx: %d. ", train_no, speed, tref, speed_idx);
				logdisplay_flushline(state->expected_time_display);
				// set up the next calibration
				speed = next_calib_speed();
				if (speed == -1) {
					engineer_set_speed(eng, train_no, 0);
					logdisplay_printf(state->expected_time_display, "calibration: finished. current train set to %d", train_no);
					logdisplay_flushline(state->expected_time_display);
					dumbbus_unregister(state->sensor_listeners, &handle_sensor_response);
					calibrator_init();
					state->cur_train = train_no;
				} else {
					engineer_set_speed(eng, train_no, speed);
					logdisplay_printf(state->expected_time_display, "next calibration: speed %d", speed);
					logdisplay_flushline(state->expected_time_display);
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
	ui_set_track(state, s, ns, c, nc);
}

void calibrate_train(a0state *state, int train_no, int min, int max) {
	engineer *eng = state->eng;

	if (calib_state.state != IDLE) {
		logstrip_printf(state->cmdlog, "train calibrator is busy, try later");
		return;
	}

	// clamp range to allowed range
	min = MAX(min, CALIB_MIN_SPEED);
	max = MIN(max, CALIB_MAX_SPEED);

	// stop the train
	engineer_set_speed(eng, train_no, 0);

	// set track to calibration mode
	calibrator_setup_track(state);

	// set currently active train
	calib_state.train_no = train_no;

	// initialize
	calib_state.state = ORIENTING;
	engineer_set_speed(eng, train_no, CALIB_ORIENTING_SPEED);
	calib_state.ascending = TRUE;
	calib_state.cur_speed = min - 1;
	calib_state.min_speed = min;
	calib_state.max_speed = max;
	calib_state.testrun = TRUE;

	// register for sensor notifications
	dumbbus_register(state->sensor_listeners, &handle_sensor_response);
}
