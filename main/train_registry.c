#include <train_registry.h>
#include <constants.h>
#include <syscall.h>
#include <util.h>

trainreg *trainreg_new() {
	trainreg *this = malloc(sizeof(trainreg));
	for (int i = TRAIN_MIN_TRAIN_ADDR; i <= TRAIN_MAX_TRAIN_ADDR; i++) {
		trainreg_add(this, i);
	}
	return this;
}

int trainreg_add(trainreg *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	train_init(&this->trains[train_no], train_no);
	return TRUE;
}

train *trainreg_get(trainreg *this, int train_no) {
	ASSERT(TRAIN_GOODNO(train_no), "bad train_no (%d)", train_no);
	return &this->trains[train_no];
}

train *trainreg_first(trainreg *this) {
	for (int i = TRAIN_MIN_TRAIN_ADDR; i <= TRAIN_MAX_TRAIN_ADDR; i++) {
		train *rv = trainreg_get(this, i);
		if (rv->calibrated) return rv;
	}
	return NULL;
}

train *trainreg_next(trainreg *this, train *cur_train) {
	int cur_train_idx = cur_train - this->trains;
	for (int i = cur_train_idx + 1; i <= TRAIN_MAX_TRAIN_ADDR; i++) {
		train *rv = trainreg_get(this, i);
		if (rv->calibrated) return rv;
	}
	return NULL;
}
