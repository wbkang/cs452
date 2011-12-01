#include <task/a0.h>
#include <dumbbus.h>
#include <string.h>
#include <train_calibrator.h>
#include <gps.h>
#include <util.h>
#include <engineer.h>

static struct {
	enum { IDLE, ORIENTING } state;
	train *train;
	track_node *front;
	track_node *back;
} cl_state;

static void handle_sensor_response();

void calibrator_init() {
	cl_state.state = IDLE;
}

static void calibrator_quit() {
	glob *state = get_glob();
	engineer *eng = state->eng;
	engineer_set_speed(eng, cl_state.train->no, 0);
	dumbbus_unregister(state->sensor_bus, &handle_sensor_response);
	calibrator_init();
	logstrip_printf(get_glob()->cmdlog, "quitting calibrating train %d", cl_state.train->no);
}

static void handle_sensor_response() {
	glob *state = get_glob();

	track_node *sensor = state->cur_sensor;
	ASSERTNOTNULL(sensor);

	train *train = cl_state.train;

	switch (cl_state.state) {
		case IDLE:
			ASSERT(0, "shouldn't be idle");
			break;
		case ORIENTING:
			if (sensor == cl_state.front) {
				train_set_dir(train, TRAIN_FORWARD);
				calibrator_quit(state);
			} else if (sensor == cl_state.back) {
				train_set_dir(train, TRAIN_BACKWARD);
				calibrator_quit(state);
			} else {
				train_set_dir(train, TRAIN_UNKNOWN);
				calibrator_quit(state);
				logstrip_printf(state->cmdlog,
					"train %d in bad position, exiting calibration.",
					train->no
				);
			}
			break;
		default:
			ASSERT(0, "bad state");
			break;
	}
}

void calibrate_train(int train_no, char sig1mod, int sig1id) {
	glob *state = get_glob();
	engineer *eng = state->eng;

	if (cl_state.state != IDLE) {
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

	track_node *back = front->reverse->edge->dest;
	if (!back) {
		logstrip_printf(state->cmdlog, "no back node for start node %c%d", sig1mod, sig1id);
		return;
	}

	// stop the train
	engineer_set_speed(eng, train_no, 0);

	// set currently active train
	cl_state.train = engineer_get_train(eng, train_no);

	// initialize
	cl_state.state = ORIENTING;
	engineer_set_speed(eng, train_no, CALIB_ORIENTING_SPEED);
	cl_state.front = front;
	cl_state.back = back;

	logstrip_printf(state->cmdlog, "orienting train %d", train_no);

	// register for sensor notifications
	dumbbus_register(state->sensor_bus, &handle_sensor_response);
}
