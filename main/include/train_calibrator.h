#pragma once
#include <task/a0.h>

#define CALIB_MIN_SPEED 8
#define CALIB_MAX_SPEED 14

static inline int calib_goodmin(int min) {
	return CALIB_MIN_SPEED <= min;
}

static inline int calib_goodmax(int max) {
	return max <= CALIB_MAX_SPEED;
}

void start_train_calibration(a0state *state, int train, int min, int max);
