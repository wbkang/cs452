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
	guard *guards[2];
};

heist *heist_new(engineer *eng, a0ui *ui) {
	heist *this = malloc(sizeof(heist));
	this->eng = eng;
	this->ui = ui;
	for (int i = 0; i < sizeof(this->guards) / sizeof (guard*); i++) {
		this->guards[i] = NULL;
	}
	return this;
}

void heist_init(heist *this, int trainno, int guardno) {
	if (guardno == 1) {
		this->guards[0] = guard_new(GUARD_1, this, this->eng, this->ui, trainno);
	} else if (guardno == 2) {
		this->guards[1] = guard_new(GUARD_2, this, this->eng, this->ui, trainno);
	} else {
		ASSERT(0, "unknown guard no %d for train %d", guardno, trainno);
	}
}

void heist_on_tick(heist *this) {
	for (int i = 0; i < sizeof(this->guards) / sizeof (guard*); i++) {
		if (this->guards[i]) {
			guard_on_tick(this->guards[i]);
		}
	}
}

void heist_set_enabled(heist *this, int enabled) {
	for (int i = 0; i < sizeof(this->guards) / sizeof (guard*); i++) {
		if (this->guards[i]) {
			guard_set_enabled(this->guards[i], enabled);
		}
	}
}
