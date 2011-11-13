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
	logstrip *log = ((a0state*) s)->stopinfo;

	int train_no = ts_state.train_no;
	location *dest = &ts_state.dest;

	location tloc;
	location *train_loc = &tloc;
	engineer_get_loc(eng, train_no, train_loc);

	if (location_isundef(train_loc)) return; // unknown train position

	fixed stop_dist = engineer_sim_stopdist(eng, train_no);
	if (fixed_sgn(stop_dist) <= 0) return; // unknown stop distance

	fixed stop_from = location_dist(train_loc, dest);
	if (fixed_sgn(stop_from) < 0) return; // bad path

	// @TODO: this should be a function of average error in position. something like nudge_dt * v / 2
	const fixed margin = fixed_new(5);

	logstrip_printf(log,
		"stop_from: %F, stop_dist: %F, margin: %F",
		stop_from,
		stop_dist,
		margin
	);

	if (fixed_cmp(stop_from, fixed_add(stop_dist, margin)) > 0) return; // wait

	engineer_set_speed(eng, train_no, 0);
	dumbbus_unregister(((a0state*) s)->time_bus, ontick);
}

// over must be in mm
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

	track_node *dest = engineer_get_tracknode(eng, type, id);
	if (!dest) {
		logstrip_printf(state->cmdlog, "Sorry. Landmark %s%d does not exist.", type, id);
		return;
	}

	ts_state.train_no = train_no;
	location *dest_loc = &ts_state.dest;
	location_init(dest_loc, track_next_edge(dest), fixed_new(over));

	if (location_isundef(dest_loc)) {
		logstrip_printf(state->cmdlog,
			"Sorry. Location %s+%dmm is invalid.",
			dest->name,
			over
		);
		return;
	}

	dumbbus_register(state->time_bus, ontick);
	logstrip_printf(state->cmdlog,
		"working on stopping train %d at %s+%Fmm",
		train_no,
		dest_loc->edge->src->name,
		dest_loc->offset
	);
}
