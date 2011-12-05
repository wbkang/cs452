#include <project/heist.h>
#include <util.h>
#include <syscall.h>
#include <engineer.h>
#include <ui/a0ui.h>
#include <train_registry.h>

#define TRAIN_UNKNOWN -1

struct heist {
	engineer *eng;
	a0ui *ui;
	int initialized;
	guard *guard1;
	guard *guard2;
};

heist *heist_new(engineer *eng, a0ui *ui) {
	heist *this = malloc(sizeof(heist));
	this->eng = eng;
	this->ui = ui;
	this->guard1 = NULL;
	this->guard2 = NULL;
	this->initialized = FALSE;
	return this;
}

void heist_init(heist *this, int guard1, int guard2) {
	this->guard1 = guard_new(GUARD_1, this, this->eng, this->ui, guard1);
//	this->guard2 = guard_new(GUARD_2, this, this->eng, this->ui, guard1);
	this->initialized = TRUE;
}

void heist_on_tick(heist *this) {
	if (!this->initialized) return;

	guard_on_tick(this->guard1);
//	guard_on_tick(this->guard2);
}
