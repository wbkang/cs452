#include <train.h>

// @TODO: remember the average velocity per edge as opposed to per track
// @TODO: fill in all velocities for at least two trains
void train_init_static(train_descriptor *train) {
	switch (train->no) {
		case 37:
			train->stopm = fixed_div(fixed_new(30838), fixed_new(10));
			train->stopb = fixed_div(fixed_new(-70501), fixed_new(100));
			train->len_pickup = fixed_new(50);
			train->dist2nose = fixed_new(22);
			train->dist2tail = fixed_new(120);
			TRAIN_FOREACH_SPEEDIDX(i) {
				train->v_avg[i] = fixed_new(0);
			}
			break;
		case 38:
			train->stopm = fixed_div(fixed_new(110993), fixed_new(100));
			train->stopb = fixed_div(fixed_new(-60299), fixed_new(1000));
			train->len_pickup = fixed_new(50);
			train->dist2nose = fixed_new(25);
			train->dist2tail = fixed_new(80);

			int data[] = {
					0, 1,
					0, 1,
					0, 1,
					0, 1,
					0, 1,
					0, 1,
					0, 1,
					0, 1,
					18520, 49713,
					21698, 50722,
					23865, 50783,
					25991, 50414,
					27665, 50045,
					30188, 51152,
					30690, 50476,
					0, 1,
					0, 1,
					0, 1,
					0, 1,
					0, 1,
					0, 1,
					0, 1,
					18311, 50783,
					20510, 50599,
					22569, 50047,
					24923, 50416,
					27130, 50477,
					29246, 50722
			};

			TRAIN_FOREACH_SPEEDIDX(i) {
				int dx = data[i * 2];
				int dt = data[i * 2 + 1];
				int v10000 = (10000 * dx) / dt;
				train->v_avg[i] = fixed_div(fixed_new(v10000), fixed_new(10000));
			}

			break;
		default:
			train->stopm = fixed_new(0);
			train->stopb = fixed_new(0);
			train->len_pickup = fixed_new(0);
			train->dist2nose = fixed_new(0);
			train->dist2tail = fixed_new(0);
			TRAIN_FOREACH_SPEEDIDX(i) {
				train->v_avg[i] = fixed_new(0);
			}
			break;
	}
}

void train_init(train_descriptor *this, int no) {
	this->no = no;
	this->dir = TRAIN_UNKNOWN;
	this->speed = 0;
	train_set_tspeed(this, 0);
	this->last_speed = 0;
	this->loc = location_undef();
	this->t_sim = 0;
	train_init_static(this);
}

fixed train_get_velocity(train_descriptor *this) {
	int speed_idx = train_get_speedidx(this);
	return this->v_avg[speed_idx];
}

fixed train_get_stopdist(train_descriptor *this) {
	fixed v = train_get_velocity(this);
	ASSERT(fixed_sgn(v) >= 0, "bad velocity for train %d", this->no);
	if (fixed_sgn(v) == 0) return fixed_new(0);
	fixed dist = fixed_add(fixed_mul(this->stopm, v), this->stopb);
	switch (this->dir) {
		case TRAIN_FORWARD:
			return fixed_add(dist, this->dist2nose);
		case TRAIN_BACKWARD:
			return fixed_add(dist, this->dist2tail);
		default:
			return dist; // @TODO: put an assert(0) here, dont run uninitialized trains
	}
}

int train_get_speed(train_descriptor *this) {
	return this->speed;
}

int train_get_speedidx(train_descriptor *this) {
	int speed = train_get_speed(this);
	if (speed <= this->last_speed || speed == TRAIN_MAX_SPEED) {
		return speed;
	} else {
		return speed + TRAIN_MAX_SPEED;
	}
}

// @TODO: the only external thing that changes a train state is the set_speed/reverse commands. we should keep a history of these commands if we want to be able to rewind the simulation.
void train_set_speed(train_descriptor *this, int speed, int t) {
	ASSERT(TRAIN_GOOD_SPEED(speed), "bad speed %d", speed);
	train_update_simulation(this, t);
	this->last_speed = this->speed;
	this->speed = speed;
	train_set_tspeed(this, t);
	this->loc = location_undef(); // lose position @TODO: remove this when get an acceleration model
}

void train_get_loc(train_descriptor *this, location *loc) {
	*loc = this->loc;
}

void train_set_loc(train_descriptor *this, location *loc) {
	this->loc = *loc;
}

train_direction train_get_dir(train_descriptor *this) {
	return this->dir;
}

void train_set_dir(train_descriptor *this, train_direction dir) {
	this->dir = dir;
}

void train_reverse_dir(train_descriptor *this) {
	switch (train_get_dir(this)) {
		case TRAIN_FORWARD:
			train_set_dir(this, TRAIN_BACKWARD);
			break;
		case TRAIN_BACKWARD:
			train_set_dir(this, TRAIN_FORWARD);
			break;
		default:
			train_set_dir(this, TRAIN_UNKNOWN);
			break;
	}
}

int train_get_tspeed(train_descriptor *this) {
	return this->t_speed;
}

void train_set_tspeed(train_descriptor *this, int t_speed) {
	this->t_speed = t_speed;
}

int train_get_tsim(train_descriptor *this) {
	return this->t_sim;
}

void train_set_tsim(train_descriptor *this, int t_sim) {
	this->t_sim = t_sim;
}

// simulate the distance the train would travel from t_i to t_f
// assuming t_i <= t_f and that t_i is relatively close to the current time
fixed train_simulate_dx(train_descriptor *this, int t_i, int t_f) {
	fixed v = train_get_velocity(this); // @TODO: don't assume current velocity
	if (fixed_sgn(v) > 0) {
		fixed dt = fixed_new(TICK2MS(t_f - t_i));
		return fixed_mul(v, dt);
	} else {
		return fixed_new(0);
	}
}

// limiting the scope of this function makes it MUCH easier to code. right now the purpose is to see where the train was at time t_i. this time is guaranteed to be near a sensor hit so there are no issues with track switch state.
// the assumption is that the switches along the path of the train did not change configuration during t_i and t_sim
// @TODO: make less assumptions about the state of the track, perhaps store the locations generated by every train_update_simulation and linearly interpolate between the closes two locations to the given t_i
void train_get_loc_hist(train_descriptor *this, int t_i, location *rv_loc) {
	int t_f = train_get_tsim(this);
	ASSERT(t_i <= t_f, "asking for non-historic data");
	fixed dx = train_simulate_dx(this, t_i, t_f);
	dx = fixed_neg(dx);
	train_get_loc(this, rv_loc);
	location_add(rv_loc, dx);
}

// @TODO: add acceleration
// @TODO: add jerk
// @TODO: modify velocity even if location is unknown
void train_update_simulation(train_descriptor *this, int t_f) {
	location loc;
	train_get_loc(this, &loc);
	if (!location_isundef(&loc)) {
		int t_i = train_get_tsim(this);
		fixed dx = train_simulate_dx(this, t_i, t_f);
		location_add(&loc, dx);
		train_set_loc(this, &loc);
	}
	train_set_tsim(this, t_f);
}
