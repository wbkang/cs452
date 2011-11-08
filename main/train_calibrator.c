#include <task/a0.h>
#include <dumbbus.h>
#include <string.h>

#define MAX_CALIB_SPEED 14
#define MIN_CALIB_SPEED 13
#define CALIB_START_SPEED MIN_CALIB_SPEED - 1

#define CALIB_SENSOR_START "E8"
#define CALIB_SENSOR_END "C14"
#define CALIB_SENSOR_WRONG_DIR "E7"

// not exactly cheating here.
static int last_train_num;
static struct {
	int ascending;
	int cur_speed;
	int train_num;
} calib_state;

static void reset_calib_state() {
	calib_state.ascending = 1;
	calib_state.cur_speed = CALIB_START_SPEED;
}

static int next_calib_speed() {
	if (calib_state.ascending) {
		if (++calib_state.cur_speed > MAX_CALIB_SPEED) {
			calib_state.cur_speed = MAX_CALIB_SPEED - 1;
			calib_state.ascending = 0;
		}
		return calib_state.cur_speed;
	} else {
		if (--calib_state.cur_speed >= MIN_CALIB_SPEED) {
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
		train_descriptor *train = &state->train[last_train_num];
		int tref = state->cur_tick - state->last_tick;

		if (train->speed != CALIB_START_SPEED) {
			int speedidx = train_speed2speed_idx(train);
			train->tref[speedidx] = tref;
			logdisplay_printf(state->landmark_display, "tref set for train %d, speed %d, to %d. speedidx:%d. ",
							last_train_num, train->speed, tref, speedidx);
		}

		// set up the next calibration
		int speed = next_calib_speed();
		if (speed != -1) {
			handle_train_speed(state, last_train_num, speed);
			logdisplay_printf(state->landmark_display, "next calibration: speed %d", speed);
			logdisplay_flushline(state->landmark_display);
		} else {
			logdisplay_printf(state->landmark_display, "calibration: finished", speed);
			logdisplay_flushline(state->landmark_display);
			dumbbus_unregister(state->sensor_listeners, &handle_sensor_response);
			handle_train_speed(state, last_train_num, 0);
			state->cur_train = calib_state.train_num;
		}
	}
}

void start_train_calibration(a0state *state, int train) {
	dumbbus_register(state->sensor_listeners, &handle_sensor_response);
	last_train_num = train;
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
	handle_train_speed(state, train, CALIB_START_SPEED);

	reset_calib_state();
}
