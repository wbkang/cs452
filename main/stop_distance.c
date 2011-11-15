#include <train.h>

// @TODO: repurpose to populate train descriptor with any static info
void populate_stop_distance(train_descriptor *train, int train_no) {
	switch (train_no) {
		case 37:
			train->stopm = fixed_div(fixed_new(30838), fixed_new(10));
			train->stopb = fixed_div(fixed_new(-70501), fixed_new(100));
			train->dist2nose = fixed_new(22);
			train->dist2tail = fixed_new(120);
			break;
		case 38:
			train->stopm = fixed_div(fixed_new(110993), fixed_new(100));
			train->stopb = fixed_div(fixed_new(-36299), fixed_new(1000));
			train->dist2nose = fixed_new(25);
			train->dist2tail = fixed_new(80);
			break;
	}
}
