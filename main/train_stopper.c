#include <train_stopper.h>
#include <task/a0.h>
#include <engineer.h>
#include <dumbbus.h>
#include <lookup.h>
#include <fixed.h>

#define DEFAULT_SEARCH_DEPTH 10

struct {
	track_node *dest;
	int over;
	int train_no;
} ts_state;

// find the closest sensor to physical stop location (without going over)
static void get_destination(track_node **rv_dest, int *rv_over) {
	track_node *node = ts_state.dest;
	int over = ts_state.over;

	for (;;) {
		track_node *next = find_next_sensor(node);
		if (!next) break;
		int dist = track_distance(node, next);
		if (dist < 0) break;
		if (dist < over) {
			node = next;
			over -= dist;
		} else {
			break;
		}
	}

	*rv_dest = node;
	*rv_over = over;
}


// logging should be handled better
// should take into account the time passed since sensor hit and now, as the train moved
static void handle_sensor(void* s) {
	a0state *state = s;
	engineer *eng = state->eng;
	int train_no = ts_state.train_no;

	// track_node *last_sensor = state->last_node;
	track_node *sensor = state->cur_node;
	track_node *dest;
	int over;
	get_destination(&dest, &over);

	int dist = track_distance(sensor, dest);
	if (dist < 0) {
		logdisplay_printf(state->log, "no path %s->%s", sensor->name, dest->name);
		logdisplay_flushline(state->log);
		return;
	}

	fixed dx = fixed_new(dist);
	fixed stopdist = engineer_sim_stopdist(eng, train_no);
	fixed offset = fixed_sub(fixed_new(over), stopdist);
	fixed stop_at = fixed_add(dx, offset);

	track_node *next_sensor = find_next_sensor(sensor);
	int dist_to_next_sensor = track_distance(sensor, next_sensor);

	if (fixed_int(stop_at) > dist_to_next_sensor && dist_to_next_sensor > 0) {
		// should wait until next sensor
		logdisplay_printf(state->log, "should wait until next sensor, dist: %d, stop_at: %F, d2ns: %d, %s->%s", dist, stop_at, dist_to_next_sensor, sensor->name, dest->name);
		logdisplay_flushline(state->log);
		return;
	}

	fixed beta = beta_sum(sensor, dest);
	if (fixed_sgn(beta) <= 0) {
		logdisplay_printf(state->log, "bad beta: %F", beta);
		logdisplay_flushline(state->log);
		return;
	}

	int speed = engineer_get_speed(eng, train_no);
	int speed_idx = engineer_get_speedidx(eng, train_no);
	fixed tref = fixed_new(engineer_get_tref(eng, train_no, speed_idx));
	if (fixed_sgn(tref) < 0) {
		logdisplay_printf(state->log, "train %d not calibrated for speed %d.", train_no, speed, speed_idx);
		logdisplay_flushline(state->log);
		return;
	}
	fixed dt = fixed_mul(beta, tref);
	if (fixed_sgn(dt) <= 0) {
		logdisplay_printf(state->log, "dt is low: %F, beta: %F, tref: %F", dt, beta, tref);
		logdisplay_flushline(state->log);
		return;
	}

	fixed v = fixed_div(dx, dt);
	int stop_ticks = fixed_int(fixed_div(stop_at, v));

	logstrip_printf(state->cmdlog, "going to stop! node: %s, sa: %F, dx: %F, dt: %F, st: %d", sensor->name, stop_at, dx, dt, stop_ticks);

	// logstrip_printf(state->cmdlog, "going to stop! stop_at: %F, d2ns: %d, stop_time: %F, dx: %F, dt: %F, v: %F, beta: %F, tref: %F", stop_at, dist_to_next_sensor, stop_time, dx, dt, v, beta, tref);

	// logstrip_printf(state->cmdlog, "should stop in %Fticks at %s. tseg: %F, dist: %d, stopdist: %F, os: %d", delay, sensor->name, tseg, dist, stopdist, overshoot_mm);
	engineer_pause_train(eng, train_no, stop_ticks);
	engineer_set_speed(eng, train_no, 0);
	dumbbus_unregister(state->sensor_bus, handle_sensor);
}

void train_stopper_setup(a0state *state, int train_no, char *type, int id, int over) {
	ASSERTNOTNULL(state);
	engineer *eng = state->eng;
	ASSERTNOTNULL(type);
	ASSERT(over >= 0, "dist negative: %d", over);

	fixed stopm, stopb;
	engineer_get_stopinfo(eng, train_no, &stopm, &stopb);
	if (fixed_is0(stopm)) {
		logstrip_printf(state->cmdlog, "Sorry. Stop distance unknown for train %d.", train_no);
		return;
	}

	// int speed = engineer_get_speed(eng, train_no);
	// int speed_idx = engineer_get_speedidx(eng, train_no);
	// int tref = engineer_get_tref(eng, train_no, speed_idx);

	track_node *dest = engineer_get_tracknode(eng, type, id);
	if (!dest) {
		logstrip_printf(state->cmdlog, "Sorry. Landmark %s%d does not exist.", type, id);
		return;
	}

	ASSERT(over >= 0, "over negative: %d", over);
	ts_state.train_no = train_no;
	ts_state.dest = dest;
	ts_state.over = 10 * over; // change to mm

	dumbbus_register(state->sensor_bus, handle_sensor);
	logstrip_printf(state->cmdlog, "will stop train %d soon.", train_no);
}
