#include <task/a0.h>
#include <dumbbus.h>
#include <string.h>
#include <train_calibrator.h>

#define CALIB_SENSOR_START "E8"
#define CALIB_SENSOR_END "C14"
#define CALIB_SENSOR_WRONG_DIR "E7"

// not exactly cheating here.
static struct {
	int min_speed;
	int max_speed;
	int ascending;
	int cur_speed;
	int train_num;
	int testrun;
} calib_state;

static void reset_calib_state(int min, int max) {
	calib_state.ascending = TRUE;
	calib_state.cur_speed = min - 1;
	calib_state.min_speed = min;
	calib_state.max_speed = max;
	calib_state.testrun = TRUE;
}

static int next_calib_speed() {
	if (calib_state.ascending) {
		if (++calib_state.cur_speed > calib_state.max_speed) {
			calib_state.cur_speed = calib_state.max_speed - 1;
			calib_state.ascending = 0;
		}
		return calib_state.cur_speed;
	} else {
		if (--calib_state.cur_speed >= calib_state.min_speed) {
			return calib_state.cur_speed;
		}
	}
	return -1;
}


static void handle_train_speed(a0state *state, int train, int speed) {
	train_descriptor *td = &state->train[train];
	td->last_speed = td->speed;
	td->speed = speed;
	train_speed(train, speed, state->tid_traincmdbuf);
}

static void handle_sensor_response(void* s) {
	a0state *state = s;
	track_node *node = state->cur_node;

	if (!(node && state->last_node)) {
		return;
	}

	if (strcmp(state->last_node->name, CALIB_SENSOR_WRONG_DIR) == 0) {
		logdisplay_printf(state->landmark_display, "Coolly reversing the train...");
		logdisplay_flushline(state->landmark_display);
//		train_reverse(last_train_num, state->tid_traincmdbuf);
		return;
	} else if (strcmp(state->last_node->name, CALIB_SENSOR_START) == 0
			&& strcmp(node->name, CALIB_SENSOR_END) == 0) {
		// process the result from the last calibration run
		train_descriptor *train = &state->train[calib_state.train_num];
		int tref = state->cur_tick - state->last_tick;

		if (!calib_state.testrun) {
			int speedidx = train_speed2speed_idx(train);
			train->tref[speedidx] = tref;
			logdisplay_printf(state->landmark_display, "tref set for train %d, speed %d, to %d. speedidx:%d. ",
					calib_state.train_num, train->speed, tref, speedidx);
		} else {
			calib_state.testrun = FALSE;
		}

		// set up the next calibration
		int speed = next_calib_speed();
		if (speed != -1) {
			handle_train_speed(state, calib_state.train_num, speed);
			logdisplay_printf(state->landmark_display, "next calibration: speed %d", speed);
			logdisplay_flushline(state->landmark_display);
		} else {
			logdisplay_printf(state->landmark_display, "calibration: finished. current train set to %d", speed, calib_state.train_num);
			logdisplay_flushline(state->landmark_display);
			dumbbus_unregister(state->sensor_listeners, &handle_sensor_response);
			handle_train_speed(state, calib_state.train_num, 0);
			state->cur_train = calib_state.train_num;
		}
	}
}

void start_train_calibration(a0state *state, int train, int min, int max) {
	ASSERT(CALIB_MIN_SPEED <= min, "calib min is %d. i can't do %d", CALIB_MIN_SPEED, min);
	ASSERT(max <= CALIB_MAX_SPEED, "calib max is %d. i can't do %d", CALIB_MAX_SPEED, max);

	dumbbus_register(state->sensor_listeners, &handle_sensor_response);
	calib_state.train_num = train;
	int s[] = { 1, 2, 4, 6, 7, 9, 14, 16, 153, 155 };
	int c[] = { 3, 5, 8, 10, 11, 12, 13, 15, 17, 18, 154, 156 };
	handle_train_speed(state, train, 0);

	for (int i = 0; i < sizeof(s)/sizeof(i); i++) {
		train_switch(s[i], 's', state->tid_traincmdbuf);
	}
	for (int i = 0; i < sizeof(c)/sizeof(i); i++) {
		train_switch(c[i], 'c', state->tid_traincmdbuf);
	}
	train_solenoidoff(state->tid_traincmdbuf);
	handle_train_speed(state, train, CALIB_MIN_SPEED - 1);
	reset_calib_state(min, max);
}
