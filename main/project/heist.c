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
	guard *guard1;
	guard *guard2;
};

heist *heist_new(engineer *eng, a0ui *ui) {
	heist *this = malloc(sizeof(heist));
	this->eng = eng;
	this->ui = ui;
	this->guard1 = NULL;
	this->guard2 = NULL;
	return this;
}

void heist_init(heist *this, int trainno, int guardno) {
	if (guardno == 1) {
		this->guard1 = guard_new(GUARD_1, this, this->eng, this->ui, trainno);
	} else if (guardno == 2) {
		this->guard2 = guard_new(GUARD_2, this, this->eng, this->ui, trainno);
	} else {
		ASSERT(0, "unknown guard no %d for train %d", guardno, trainno);
	}
}

void heist_on_tick(heist *this) {
	if (this->guard1) guard_on_tick(this->guard1);
	if (this->guard2) guard_on_tick(this->guard2);
}
