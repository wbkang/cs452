#pragma once
#include <engineer.h>
#include <ui/a0ui.h>

typedef struct heist heist;

heist *heist_new(engineer *eng, a0ui *ui);
void heist_init(heist *heist, int guard1, int guard2);
