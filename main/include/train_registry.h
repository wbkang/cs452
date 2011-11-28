#pragma once

#include <train.h>

typedef struct trainreg trainreg;
struct trainreg {
	train trains[TRAIN_MAX_TRAIN_ADDR + 1];
};

trainreg *trainreg_new();
int trainreg_add(trainreg *this, int train_no);
train *trainreg_get(trainreg *this, int train_no);
train *trainreg_first(trainreg *this);
train *trainreg_next(trainreg *this, train *cur_train);

#define trainreg_foreach(this, x) \
	for (train *(x) = trainreg_first(this); (x); (x) = trainreg_next(this, (x)))
