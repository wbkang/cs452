#include <train_stopper.h>
#include <task/a0.h>
#include <dumbbus.h>
#include <lookup.h>
#include <fixed.h>

#define DEFAULT_SEARCH_DEPTH 10

struct {
	track_node *target;
	int dist_cm;
	train_descriptor *train;
} ts_state;

static void handle_sensor(void* s) {
	a0state *state = s;

	track_node *curnode = state->cur_node;
	track_node *target = ts_state.target;

	int dist = find_dist(curnode, target, 0, DEFAULT_SEARCH_DEPTH);

	if (dist == -1) {
		logstrip_printf(state->cmdlog, "no dist found between %s->%s", curnode->name, target->name);
		return;
	}

	train_descriptor *train = ts_state.train;
	fixed speed = fixed_new(train->speed);
	int overshoot_mm =  10 * ts_state.dist_cm;
	int overshot_target_dist = dist + overshoot_mm;
	fixed stopdist = fixed_add(fixed_mul(train->stop_dist_slope, speed), train->stop_dist_offset);

	track_edge *nextedge = find_forward(curnode);
	track_node *nextsensor = find_next_sensor(curnode);
	int dist_to_next_sensor = nextsensor ? find_dist(curnode, nextsensor, 0, 2) : -1;

	if (dist_to_next_sensor != -1) {
		int maxstopdist = fixed_int(fixed_add(fixed_new(dist_to_next_sensor), stopdist));

		if (overshot_target_dist >= maxstopdist) {
			logstrip_printf(state->cmdlog, "can't stop %s. slope:%F, offset:%F stopdist: %F, dist_to_next_sensor: %d",
					curnode->name, train->stop_dist_slope, train->stop_dist_offset, stopdist, dist_to_next_sensor);
			return;
		}
	}

	// (s - (d - x)) /s * t_seg
	// where s = dist to the target, x, the overshoot, d, stopping dist
	fixed r = dist ? fixed_div(
						fixed_sub(
							fixed_new(dist),
							fixed_sub(stopdist, fixed_new(overshoot_mm))),
						fixed_new(dist)): 0;
	fixed tref = fixed_new(train->tref[train_speed2speed_idx(train)]);
	fixed tseg = fixed_mul(tref, nextedge->beta);

	fixed delay = fixed_mul(r, tseg);

	logstrip_printf(state->cmdlog, "should stop in %Fticks at %s. tseg:%F, dist:%d, stopdist:%F, os:%d",
			delay, curnode->name, tseg, dist, stopdist, overshoot_mm);

	if (fixed_int(delay) > 0) {
		traincmdbuffer_put(state->tid_traincmdbuf, PAUSE, fixed_int(delay), NULL);
	}
	train_speed(state->cur_train, 0, state->tid_traincmdbuf);
	dumbbus_unregister(state->sensor_listeners, handle_sensor);
}

void train_stopper_setup(a0state *state, int trainno, char *word, int modnum, int dist_cm) {
	ASSERTNOTNULL(state);
	ASSERTNOTNULL(word);
	ASSERT(dist_cm >= 0, "dist negative: %d", dist_cm);

	train_descriptor *train = &state->train[trainno];
	if (train->stop_dist_slope == 0) {
		logstrip_printf(state->cmdlog, "Sorry. Stop distance unknown for train %d.", trainno);
		return;
	}

	if (train->tref[train->speed] == -1) {
		logstrip_printf(state->cmdlog, "Sorry. train %d not calibrated.", trainno);
		return;
	}

	char modname[8];
	sprintf(modname, "%s%d", word, modnum);
	track_node *target = lookup_get(state->nodemap, modname);
	if(!target) {
		logstrip_printf(state->cmdlog, "Sorry. Landmark %s does not exist.", modname);
		return;
	}

	ASSERT(dist_cm >= 0, "dist_cm negative: %d", dist_cm);
	ts_state.train = train;
	ts_state.target = target;
	ts_state.dist_cm = dist_cm;

	dumbbus_register(state->sensor_listeners, handle_sensor);
	logstrip_printf(state->cmdlog, "will think about stopping the train %d soon.", trainno);
}
