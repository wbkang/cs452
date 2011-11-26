#include <train_stopper.h>
#include <task/a0.h>
#include <engineer.h>
#include <dumbbus.h>
#include <lookup.h>
#include <fixed.h>

#define DEFAULT_SEARCH_DEPTH 10

struct {
	int train_no;
	location dest;
} ts_state;

// @TODO: logging should be handled better
static void ontick(void* s) {
	engineer *eng = ((a0state*) s)->eng;

	int train_no = ts_state.train_no;
	location *dest = &ts_state.dest;

	location tloc;
	location *train_loc = &tloc;
	engineer_get_loc(eng, train_no, train_loc);

	if (location_isundef(train_loc)) return; // unknown train position

	int stop_dist = engineer_sim_stopdist(eng, train_no);
	if (stop_dist < 0) return; // unknown stop distance

	location stat = tloc;
	location_add(&stat, fixed_new(stop_dist));

	int stop_from = location_dist_dir(train_loc, dest);
	if (stop_from < 0) return; // bad path

	// @TODO: this should be a function of average error in position. something like nudge_dt * v / 2

	if (abs(stop_from - stop_dist) > 5) return; // wait

	engineer_set_speed(eng, train_no, 0);
	dumbbus_unregister(((a0state*) s)->simbus, ontick);

	logstrip_printf(((a0state*) s)->cmdlog,
		"stopping train %d",
		train_no
	);
}

// over must be in mm
void train_stopper_setup(a0state *state, int train_no, char *type, int id, int over) {
	ASSERTNOTNULL(state);
	engineer *eng = state->eng;
	ASSERTNOTNULL(type);
	ASSERT(over >= 0, "dist negative: %d", over);

	track_node *dest = engineer_get_tracknode(eng, type, id);
	if (!dest) {
		logstrip_printf(state->cmdlog, "Sorry. Landmark %s%d does not exist.", type, id);
		return;
	}

	ts_state.train_no = train_no;
	ts_state.dest = location_fromnode(dest, 0);
	location_add(&ts_state.dest, fixed_new(over));

	if (location_isundef(&ts_state.dest)) {
		logstrip_printf(state->cmdlog,
			"Sorry. Location %s+%dmm is invalid.",
			dest->name,
			over
		);
		return;
	}

	dumbbus_register(state->simbus, ontick);
	logstrip_printf(state->cmdlog,
		"working on stopping train %d at %s+%dmm",
		train_no,
		dest->name,
		over
	);
}
