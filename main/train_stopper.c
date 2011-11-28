#include <train_stopper.h>
#include <task/a0.h>
#include <engineer.h>
#include <dumbbus.h>
#include <lookup.h>
#include <fixed.h>

#define DEFAULT_SEARCH_DEPTH 10

struct {
	train *train;
	location loc_dest;
} ts_state;

// @TODO: logging should be handled better
static void ontick(void* s) {
	train *train = ts_state.train;
	if (train_is_lost(train)) return;

	location loc_train = train_get_frontloc(train);
	location *loc_dest = &ts_state.loc_dest;
	int stop_dist = train_get_stopdist(train);

	if (location_dist_dir(&loc_train, loc_dest) > stop_dist) return;

	a0state *state = s;
	engineer_set_speed(state->eng, train->no, 0);
	dumbbus_unregister(state->simbus, ontick);
	logstrip_printf(state->cmdlog, "stopping train %d", train->no);
}

// over must be in mm
void train_stopper_setup(a0state *state, int train_no, char *type, int id, int over) {
	ASSERTNOTNULL(state);
	engineer *eng = state->eng;
	ASSERTNOTNULL(type);
	ASSERT(over >= 0, "dist negative: %d", over);

	track_node *loc_dest = engineer_get_tracknode(eng, type, id);
	if (!loc_dest) {
		logstrip_printf(state->cmdlog, "Sorry. Landmark %s%d does not exist.", type, id);
		return;
	}

	ts_state.train = engineer_get_train(eng, train_no);
	ts_state.loc_dest = location_fromnode(loc_dest, 0);
	location_add(&ts_state.loc_dest, fixed_new(over));

	if (location_isundef(&ts_state.loc_dest)) {
		logstrip_printf(state->cmdlog,
			"Sorry. Location %s+%dmm is invalid.",
			loc_dest->name,
			over
		);
		return;
	}

	dumbbus_register(state->simbus, ontick);
	logstrip_printf(state->cmdlog,
		"working on stopping train %d at %s+%dmm",
		train_no,
		loc_dest->name,
		over
	);
}
