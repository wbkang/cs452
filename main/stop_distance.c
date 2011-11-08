#include <train.h>

void populate_stop_distance(train_descriptor *train, int trainnum) {
	switch(trainnum) {
		case 38:
			train->stop_dist_slope = fixed_div(fixed_new(54703), fixed_new(1000));
			//14228 - 7500
			train->stop_dist_offset = fixed_div(fixed_new(6728), fixed_new(1000));
			break;
	}
}
