#include <train_stopper.h>
#include <task/a0.h>
#include <engineer.h>
#include <dumbbus.h>
#include <lookup.h>

#define DEFAULT_SEARCH_DEPTH 10

struct {
	train *train;
	location loc_dest;
} ts_state;

void train_stopper_setup(dumbbus *simbus, train *train, location *dest) {
	ts_state.train = train;
	ts_state.loc_dest = *dest;
	dumbbus_register(simbus, train_stopper_on_tick);
}

void train_stopper_on_tick(void *vengineer, void *va0) {
	a0state *a0state = va0;
	train *train = ts_state.train;
	if (train_is_lost(train)) return;

	location loc_train = train_get_frontloc(train);
	location *loc_dest = &ts_state.loc_dest;
	int stop_dist = train_get_stopdist(train);

	if (location_dist_dir(&loc_train, loc_dest) > stop_dist) return;

	engineer_set_speed(a0state->eng, train->no, 0);
	dumbbus_unregister(a0state->simbus, train_stopper_on_tick);
	a0ui_on_cmdlogf(a0state->a0ui, "stopping train %d", train->no);
}

