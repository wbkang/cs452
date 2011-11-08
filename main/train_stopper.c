#include <train_stopper.h>
#include <task/a0.h>
#include <engineer.h>
#include <dumbbus.h>
#include <lookup.h>
#include <fixed.h>

#define DEFAULT_SEARCH_DEPTH 10

struct {
	track_node *dest;
	int dist_cm;
	int train_no;
} ts_state;

static void handle_sensor(void* s) {
	a0state *state = s;
	engineer *eng = state->eng;

	track_node *curnode = state->cur_node;
	track_node *dest = ts_state.dest;

	int dist = find_dist(curnode, dest, 0, DEFAULT_SEARCH_DEPTH);

	if (dist == -1) {
		logstrip_printf(state->cmdlog, "no dist found between %s->%s", curnode->name, dest->name);
		return;
	}

	int train_no = ts_state.train_no;

	int overshoot_mm =  10 * ts_state.dist_cm;
	int overshot_dest_dist = dist + overshoot_mm;

	fixed stopm, stopb;
	engineer_get_stopinfo(eng, train_no, &stopm, &stopb);

	// @TODO: take into account the velocity instead
	int speed = engineer_get_speed(eng, train_no);
	fixed stopdist = fixed_add(fixed_mul(stopm, fixed_new(speed)), stopb);

	track_edge *nextedge = find_forward(curnode);
	track_node *nextsensor = find_next_sensor(curnode);
	int dist_to_next_sensor = find_dist(curnode, nextsensor, 0, 2);

	if (dist_to_next_sensor != -1) {
		int maxstopdist = fixed_int(fixed_add(fixed_new(dist_to_next_sensor), stopdist));

		if (overshot_dest_dist >= maxstopdist) {
			logstrip_printf(state->cmdlog, "can't stop %s. slope:%F, offset:%F stopdist: %F, dist_to_next_sensor: %d",
					curnode->name, stopm, stopb, stopdist, dist_to_next_sensor);
			return;
		}
	}

	// (s - (d - x)) /s * t_seg
	// where s = dist to the dest, x, the overshoot, d, stopping dist
	fixed r = dist ? fixed_div(
						fixed_sub(
							fixed_new(dist),
							fixed_sub(stopdist, fixed_new(overshoot_mm))),
						fixed_new(dist)): 0;
	int speed_idx = engineer_get_speedidx(eng, train_no);

	fixed tref = fixed_new(engineer_get_tref(eng, train_no, speed_idx));
	fixed tseg = fixed_mul(tref, nextedge->beta);

	fixed delay = fixed_mul(r, tseg);

	logstrip_printf(state->cmdlog, "should stop in %Fticks at %s. tseg:%F, dist:%d, stopdist:%F, os:%d",
			delay, curnode->name, tseg, dist, stopdist, overshoot_mm);

	if (fixed_int(delay) > 0) {
		engineer_pause_train(eng, train_no, fixed_int(delay));
	}
	engineer_set_speed(eng, train_no, 0);
	dumbbus_unregister(state->sensor_listeners, handle_sensor);
}

void train_stopper_setup(a0state *state, int train_no, char *type, int id, int dist_cm) {
	ASSERTNOTNULL(state);
	engineer *eng = state->eng;
	ASSERTNOTNULL(type);
	ASSERT(dist_cm >= 0, "dist negative: %d", dist_cm);

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

	ASSERT(dist_cm >= 0, "dist_cm negative: %d", dist_cm);
	ts_state.train_no = train_no;
	ts_state.dest = dest;
	ts_state.dist_cm = dist_cm;

	dumbbus_register(state->sensor_listeners, handle_sensor);
	logstrip_printf(state->cmdlog, "will stop train %d soon.", train_no);
}
