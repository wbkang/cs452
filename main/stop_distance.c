#include <train.h>

void populate_stop_distance(train_descriptor *train, int trainnum) {
	switch (trainnum) {
		case 38:
			train->stopm = fixed_div(fixed_new(54703), fixed_new(1000));
			train->stopb = fixed_div(fixed_new(-60771), fixed_new(1000));
			break;
	}
}
