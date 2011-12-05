#include <project/heist.h>
#include <project/guard.h>
#include <syscall.h>
#include <track_node.h>

static char *figure_eight_loop[] = { "E2", "C2", "D2", "B13" };
static char *outer_loop[] = { "E8", "A15", "A11", "E11", "D8" };


typedef struct loopdata {
	char **loop;
	int len;
} loopdata;

guard* guard_new(guardtype type, struct heist *heist, engineer *eng, a0ui *ui, int train_no) {
	guard *this = malloc(sizeof(guard));
	this->eng = eng;
	this->type = type;
	this->heist = heist;
	this->ui = ui;
	this->train = engineer_get_train(eng, train_no);
	this->state = 0;

	if (type == GUARD_1) {
		a0ui_on_logf(this->ui, "guard1 created. train no %d.", train_no);
		this->guardproc = loopguard_on_tick;
		this->train->reversible = FALSE;
		this->train->stopatdest = FALSE;
		loopdata *data = malloc(sizeof(loopdata));
		data->loop = figure_eight_loop;
		data->len = sizeof(figure_eight_loop) / sizeof(char*);
		this->data = data;
	} else if (type == GUARD_2) {
		a0ui_on_logf(this->ui, "guard2 created. train no %d.", train_no);
		this->guardproc = loopguard_on_tick;
		this->train->reversible = FALSE;
		this->train->stopatdest = FALSE;
		loopdata *data = malloc(sizeof(loopdata));
		data->loop = outer_loop;
		data->len = sizeof(outer_loop) / sizeof(char*);
		this->data = data;
	} else {
		ASSERT(0, "unknown guard type %d", type);
	}
	return this;
}

void guard_on_tick(guard *this) {
	this->guardproc(this);
}

void loopguard_on_tick(guard *this) {
	location curdest = train_get_dest(this->train);
	loopdata *loopdata = this->data;

	if (!train_is_lost(this->train) && location_isundef(&curdest)) {
		track_node * nextnode = engineer_get_tracknode_str(this->eng, loopdata->loop[this->state]);
		location newdest = location_fromnode(nextnode, 0);
		char locname[64];
		location_tostring(&newdest, locname);
		a0ui_on_logf(this->ui, "guard%d(%d): setting the dest to %s.", this->type + 1, this->train->no, locname);
		train_set_dest(this->train, &newdest);
		this->state++;
		this->state %= loopdata->len;
	}
}
