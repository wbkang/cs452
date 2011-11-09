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

static void handle_sensor(void* s) {
	a0state *state = s;
	engineer *eng = state->eng;
	int train_no = ts_state.train_no;

	// track_node *last_sensor = state->last_node;
	track_node *sensor = state->cur_node;
	track_node *dest = ts_state.dest;

	int dist = find_dist(sensor, dest, 0, DEFAULT_SEARCH_DEPTH);
	if (dist <= 0) {
		logdisplay_printf(state->expected_time_display, "bad path %s->%s", sensor->name, dest->name);
		logdisplay_flushline(state->expected_time_display);
		return;
	}
	fixed dx = fixed_new(dist);

	fixed stopm, stopb;
	engineer_get_stopinfo(eng, train_no, &stopm, &stopb);

	// @TODO: use velocity instead
	int speed = engineer_get_speed(eng, train_no);
	fixed stopdist = fixed_add(fixed_mul(stopm, fixed_new(speed)), stopb);

	fixed offset = fixed_sub(fixed_new(ts_state.over), stopdist);
	fixed stop_at = fixed_add(dx, offset);

	// if (fixed_new(dist) + 50 < stopdist) { // definitely not enough time to stop
	// 	logstrip_printf(state->cmdlog, "not enough time to stop, dist: %F, stopdist: %F", dist, stopdist);
	// 	return;
	// }

	// track_edge *nextedge = find_forward(sensor);
	track_node *next_sensor = find_next_sensor(sensor);
	int dist_to_next_sensor = find_dist(sensor, next_sensor, 0, DEFAULT_SEARCH_DEPTH);

	// int maxstopdist = fixed_int(fixed_add(fixed_new(dist_to_next_sensor), stopdist));

	if (fixed_int(stop_at) > dist_to_next_sensor && dist_to_next_sensor > 0) { // should wait until next sensor
		logdisplay_printf(state->expected_time_display, "should wait until next sensor, dist: %d, stop_at: %F, d2ns: %d, %s->%s", dist, stop_at, dist_to_next_sensor, sensor->name, dest->name);
		logdisplay_flushline(state->expected_time_display);
		// logstrip_printf(state->cmdlog, "should wait until next sensor, dist: %d, offset: %F, stopdist: %F, stop_at: %F, d2ns: %d, %s->%s", dist, offset, stopdist, stop_at, dist_to_next_sensor, sensor->name, next_sensor->name);
		return;
	}

	fixed beta = beta_sum(sensor, dest);
	if (beta <= 0) {
		logdisplay_printf(state->expected_time_display, "bad beta: %F", beta);
		logdisplay_flushline(state->expected_time_display);
		return;
	}
	fixed speed_idx = engineer_get_speedidx(eng, train_no);
	fixed tref = fixed_new(engineer_get_tref(eng, train_no, speed_idx));

	// we can try averaging the velocity here
	track_node *last_node = state->last_node;
	fixed dx_last = find_dist(last_node, sensor, 0, 1);
	if (dx_last != -1) {
		fixed beta_last = beta_sum(last_node, sensor);
		if (beta_last >= 0) {
			dx = fixed_add(dx, dx_last);
			beta = fixed_add(beta, beta_last);
		}
	}

	fixed dt = fixed_mul(beta, tref);
	if (dt <= 0) {
		logdisplay_printf(state->expected_time_display, "dt is low: %F, beta: %F, tref: %F", dt, beta, tref);
		logdisplay_flushline(state->expected_time_display);
		return;
	}

//	int stop_ticks = fixed_int(fixed_div(fixed_mul(stop_at, dt), dx)); TODO overflow's a bitch
	int stop_ticks = fixed_int(fixed_mul(fixed_div(stop_at, dx), dt));

	logstrip_printf(state->cmdlog, "going to stop! node: %s, sa: %F, dx: %F, dt: %F, st: %d",
			sensor->name, stop_at, dx, dt, stop_ticks);

	// logstrip_printf(state->cmdlog, "going to stop! stop_at: %F, d2ns: %d, stop_time: %F, dx: %F, dt: %F, v: %F, beta: %F, tref: %F", stop_at, dist_to_next_sensor, stop_time, dx, dt, v, beta, tref);

	// fixed r = fixed_div(stop_at, dist);

	// (s - (d - x)) /s * t_seg
	// where s = dist to the dest, x, the overshoot, d, stopping dist
	// fixed r = dist ? fixed_div(
	// 					fixed_sub(
	// 						fixed_new(dist),
	// 						fixed_sub(stopdist, fixed_new(overshoot_mm))),
	// 					fixed_new(dist)): 0;
	// int speed_idx = engineer_get_speedidx(eng, train_no);

	// fixed tref = fixed_new(engineer_get_tref(eng, train_no, speed_idx));
	// fixed tseg = fixed_mul(tref, nextedge->beta);

	// fixed delay = fixed_mul(r, tseg);

	// logstrip_printf(state->cmdlog, "should stop in %Fticks at %s. tseg:%F, dist:%d, stopdist:%F, os:%d", delay, sensor->name, tseg, dist, stopdist, overshoot_mm);

	if (stop_ticks > 0) {
		engineer_pause_train(eng, train_no, stop_ticks);
	}
	engineer_set_speed(eng, train_no, 0);
	dumbbus_unregister(state->sensor_listeners, handle_sensor);
}

void train_stopper_setup(a0state *state, int train_no, char *type, int id, int over) {
	ASSERTNOTNULL(state);
	engineer *eng = state->eng;
	ASSERTNOTNULL(type);
	ASSERT(over >= 0, "dist negative: %d", over);

	fixed stopm, stopb;
	engineer_get_stopinfo(eng, train_no, &stopm, &stopb);
	if (!stopm) {
		logstrip_printf(state->cmdlog, "Sorry. Stop distance unknown for train %d.", train_no);
		return;
	}

	int speed = engineer_get_speed(eng, train_no);
	int speed_idx = engineer_get_speedidx(eng, train_no);
	int tref = engineer_get_tref(eng, train_no, speed_idx);

	if (tref == -1) {
		logstrip_printf(state->cmdlog, "Sorry. train %d not calibrated for speed %d.", train_no, speed);
		return;
	}

	track_node *dest = engineer_get_tracknode(eng, type, id);
	if (!dest) {
		logstrip_printf(state->cmdlog, "Sorry. Landmark %s%d does not exist.", type, id);
		return;
	}

	ASSERT(over >= 0, "over negative: %d", over);
	ts_state.train_no = train_no;
	ts_state.dest = dest;
	ts_state.over = 10 * over; // change to mm

	dumbbus_register(state->sensor_listeners, handle_sensor);
	logstrip_printf(state->cmdlog, "will stop train %d soon.", train_no);
}
