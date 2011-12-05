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
	int guard1;
	int guard2;
};

heist *heist_new(engineer *eng, a0ui *ui) {
	heist *this = malloc(sizeof(heist));
	this->eng = eng;
	this->ui = ui;
	this->guard1 = TRAIN_UNKNOWN;
	this->guard2 = TRAIN_UNKNOWN;
	this->initialized = FALSE;
	return this;
}

void heist_init(heist *this, int guard1, int guard2) {
	this->guard1 = guard1;
	this->guard2 = guard2;
	this->initialized = TRUE;
}

void heist_on_tick(heist *this) {
	a0ui_on_logf(this->ui, "heist on tick initialized:%d", this->initialized);
	if (!this->initialized) return;
}
