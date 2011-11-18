#include <train.h>

// @TODO: repurpose to populate train descriptor with any static info
void train_data_populate(train_descriptor *train, int train_no) {
	switch (train_no) {
		case 37:
			train->stopm = fixed_div(fixed_new(30838), fixed_new(10));
			train->stopb = fixed_div(fixed_new(-70501), fixed_new(100));
			train->dist2nose = fixed_new(22);
			train->dist2tail = fixed_new(120);
			break;
		case 38:
			train->stopm = fixed_div(fixed_new(110993), fixed_new(100));
			train->stopb = fixed_div(fixed_new(-60299), fixed_new(1000));
			train->dist2nose = fixed_new(25);
			train->dist2tail = fixed_new(80);

			int eightto14[] = {
					19232, 50292,
					21698, 50722,
					23865, 50783,
					25991, 50414,
					27665, 50045,
					30188, 51152,
					30690, 50476
			};

			for (int i = 8; i <= 14; i++) {
				train->v_avg_d[i] = eightto14[(i - 8) * 2];
				train->v_avg_t[i] = eightto14[(i - 8) * 2 + 1];
			}

			int twentytwoto27[] = {
					18311,	50783,
					20510,	50599,
					22569,	50047,
					24923,	50416,
					27130,	50477,
					29246,	50722
			};

			for (int i = 22; i <= 27; i++) {
				train->v_avg_d[i] = twentytwoto27[(i - 22) * 2];
				train->v_avg_t[i] = twentytwoto27[(i - 22) * 2 + 1];
			}

			break;
	}
}
