#include <project/heist.h>
#include <project/guard.h>
#include <syscall.h>

guard* guard_new(guardtype type, struct heist *heist, engineer *eng, a0ui *ui, int train_no) {
	guard *this = malloc(sizeof(guard));
	this->eng = eng;
	this->type = type;
	this->heist = heist;
	this->ui = ui;
	this->train_no = train_no;
	this->guardproc = guard1_on_tick;
	return this;
}

void guard1_on_tick(guard *this) {
	a0ui_on_log(this->ui, "guard1_on_tick");
}

void guard_on_tick(guard *this) {
	this->guardproc(this);
}

