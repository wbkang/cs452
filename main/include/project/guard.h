#pragma once

#include <project/heist.h>
#include <ui/a0ui.h>
#include <engineer.h>

struct heist;
struct guard;

typedef enum guardtype { GUARD_1, GUARD_2 } guardtype;
typedef void (*guardproc)(struct guard *this);


typedef struct guard {
	struct heist *heist;
	guardtype type;
	a0ui *ui;
	train* train;
	engineer *eng;
	guardproc guardproc;
	guardproc guardenableproc;
	int state;
	void *data;
	int enabled;
} guard;


guard* guard_new(guardtype type, struct heist *heist, engineer *eng, a0ui *ui, int train_no);
void guard_set_enabled(guard *this, int enabled);
void guard_on_tick(guard *this);
void loopguard_set_enable(guard *this);
void loopguard_on_tick(guard *this);
