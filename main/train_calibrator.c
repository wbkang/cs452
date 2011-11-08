#include <task/a0.h>
#include <dumbbus.h>
#include <string.h>

#define MAX_CALIB_SPEED 14
#define MIN_CALIB_SPEED 8

#define CALIB_SENSOR_START "E8"
#define CALIB_SENSOR_END "C14"

// not exactly cheating here.
static int last_train_num;
static int last_calib_speed;

static void handle_sensor_response(void* s) {
	a0state *state = s;
	track_node *node = state->cur_node;

	if (!(node && state->last_node)) {
		logdisplay_printf(state->landmark_display, "one of the nodes are null. %d", state->cur_tick);
		logdisplay_flushline(state->landmark_display);
		return;
	}

	if (strcmp(state->last_node->name, CALIB_SENSOR_START) == 0
		&& strcmp(node->name, CALIB_SENSOR_END) == 0) {
		if (last_calib_speed <= MAX_CALIB_SPEED) {
			state->train_desc[last_train_num].tref[last_calib_speed] = fixed_new(state->cur_tick - state->last_tick);

			logdisplay_printf(state->landmark_display, "tref set for train %d, speed %d, to %F",
					last_train_num, last_calib_speed, fixed_new(state->cur_tick - state->last_tick));
			logdisplay_flushline(state->landmark_display);
		}

		last_calib_speed--;
		train_speed(last_train_num, last_calib_speed, state->tid_traincmdbuf);
	}

	if (last_calib_speed < MIN_CALIB_SPEED) {
		dumbbus_unregister(state->sensor_listeners, &handle_sensor_response);
		train_speed(last_train_num, 0, state->tid_traincmdbuf);
	}
}

void start_train_calibration(a0state *state, int train) {
	dumbbus_register(state->sensor_listeners, &handle_sensor_response);
	last_train_num = train;
	last_calib_speed = MAX_CALIB_SPEED + 1;
	train_speed(train, MAX_CALIB_SPEED, state->tid_traincmdbuf);
}
