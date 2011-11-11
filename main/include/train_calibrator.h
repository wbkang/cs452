#pragma once
#include <task/a0.h>

#define CALIB_MIN_SPEED 4
#define CALIB_MAX_SPEED 14
#define CALIB_ORIENTING_SPEED CALIB_MAX_SPEED

static inline int calib_goodmin(int min) {
	return CALIB_MIN_SPEED <= min;
}

static inline int calib_goodmax(int max) {
	return max <= CALIB_MAX_SPEED;
}

void calibrator_init();

void calibrate_train(a0state *state, int train, int min, int max);
