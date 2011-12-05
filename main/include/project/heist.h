#pragma once
#include <engineer.h>
#include <ui/a0ui.h>
#include <project/guard.h>

typedef struct heist heist;
struct guard;

heist *heist_new(engineer *eng, a0ui *ui);
void heist_init(heist *heist, int guard1, int guard2);
void heist_on_tick(heist *this);
void heist_set_enabled(heist *this, int enabled);
