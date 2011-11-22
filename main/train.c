#include <train.h>
#include <gps.h>
#include <lookup.h>
#include <string.h>
#include <ui/logdisplay.h>

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
			train->ai_avg10000 = fixed_new(0);
			train->ad_avg10000 = fixed_new(0);
			train->calibrated = FALSE;
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

			// train->ai_avg10000 = fixed_div(fixed_new(49669), fixed_new(10000));
			train->ai_avg10000 = fixed_div(fixed_new(20101), fixed_new(10000));
			train->ad_avg10000 = fixed_div(fixed_new(-20101), fixed_new(10000));
			train->calibrated = TRUE;
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
			train->ai_avg10000 = fixed_new(0);
			train->ad_avg10000 = fixed_new(0);
			train->calibrated = FALSE;
			break;
	}
}

void train_init(train_descriptor *this, int no, gps *gps) {
	this->no = no;
	this->dir = TRAIN_UNKNOWN;
	this->v10000 = fixed_new(0);
	this->v_i10000 = fixed_new(0);
	this->v_f10000 = fixed_new(0);
	this->speed = 0;
	train_set_tspeed(this, 0);
	this->last_speed = 0;
	this->loc = location_undef();
	this->t_sim = 0;
	this->gps = gps;
	this->vcmds = NULL;
	this->vcmdidx = 0;
	this->vcmdslen = 0;
	this->destination = location_undef();
	train_init_static(this);
}

fixed train_get_velocity(train_descriptor *this) {
	return fixed_div(this->v10000, fixed_new(10000));
}

fixed train_get_stopdist(train_descriptor *this) {
	return train_get_stopdist4speedidx(this, train_get_speedidx(this));
}

fixed train_get_stopdist4speedidx(train_descriptor *this, int speed_idx) {
	fixed v = this->v_avg[speed_idx];
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
	return train_speed2speedidx(this->last_speed, this->speed);
}

// @TODO: the only external thing that changes a train state is the set_speed/reverse commands. we should keep a history of these commands if we want to be able to rewind the simulation.
void train_set_speed(train_descriptor *this, int speed, int t) {
	ASSERT(TRAIN_GOOD_SPEED(speed), "bad speed %d", speed);
	train_update_simulation(this, t);
	this->last_speed = this->speed;
	this->speed = speed;
	train_set_tspeed(this, t);
	this->v_i10000 = this->v10000;
	this->v_f10000 = fixed_mul(this->v_avg[train_get_speedidx(this)], fixed_new(10000));
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

void train_on_reverse(train_descriptor *this) {
	train_reverse_dir(this);
	location_reverse(&this->loc);
	location_add(&this->loc, this->len_pickup);
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

// simulation step is 2^TRAIN_SIM_DT_EXP milliseconds
#define TRAIN_SIM_DT_EXP 2
#define TRAIN_SIM_DT (1 << TRAIN_SIM_DT_EXP)

// @TODO: add jerk
static void train_step_sim(train_descriptor *this, int dt) {
	const fixed margin = fixed_new(1);
	fixed diff = fixed_sub(this->v10000, this->v_f10000);
	fixed fdt = fixed_new(dt);
	if (fixed_cmp(fixed_abs(diff), margin) > 0) {
		if (fixed_sgn(diff) < 0) {
			this->v10000 = fixed_add(this->v10000, fixed_mul(this->ai_avg10000, fdt));
		} else {
			this->v10000 = fixed_add(this->v10000, fixed_mul(this->ad_avg10000, fdt));
		}
	}
	if (!location_isundef(&this->loc) && fixed_cmp(fixed_abs(this->v10000), margin) > 0) {
		fixed dx10000 = fixed_mul(this->v10000, fdt);
		fixed dx = fixed_div(dx10000, fixed_new(10000));
		location_add(&this->loc, dx);
	}
	this->t_sim += dt;
}

void train_update_simulation(train_descriptor *this, int t_f) {
	int t_i = train_get_tsim(this);
	// train_step_sim(this, fixed_new(t_f - t_i));
	int time = TICK2MS(t_f - t_i);
//	train_step_sim(this, time);
	for (int i = (time >> TRAIN_SIM_DT_EXP); i > 0; --i) {
		train_step_sim(this, TRAIN_SIM_DT);
	}
	train_step_sim(this, time & (TRAIN_SIM_DT - 1));
}

void train_set_dest(train_descriptor *this, location *dest) {
	this->destination = *dest;
}

static trainvcmd *lastvcmd;

void train_run_vcmd(train_descriptor *this, int tid_traincmdbuf, lookup *nodemap, logdisplay *log) {
	// TODO this is a weird criteria. fix this
	if (!location_isundef(&this->destination) && this->vcmdslen == 0) {
		char buf[100]; location2str(buf, &this->destination);
//		ASSERT(0, "location %s", buf);
		if (this->vcmds == NULL) {
			this->vcmds = malloc(sizeof(trainvcmd) * TRAIN_MAX_VCMD);
		}
		gps_findpath(this->gps, this, &this->destination, TRAIN_MAX_VCMD, this->vcmds, &this->vcmdslen);
		lastvcmd = NULL;

		for (int i = 0; i < this->vcmdslen; i++) {
			char vcmdname[100];
			vcmd2str(vcmdname, &this->vcmds[i]);
//			PRINT("trip plan %d: %s", i, vcmdname);
			logdisplay_printf(log, "trip plan %d: %s", i, vcmdname);
			logdisplay_flushline(log);
		}
//		ExitKernel(0);
		this->vcmdidx = 0;
	}

	while (this->vcmdidx < this->vcmdslen) {
		trainvcmd *curvcmd = &this->vcmds[this->vcmdidx];
		char vcmdname[100];
		vcmd2str(vcmdname, curvcmd);

//		if (curvcmd != lastvcmd) {
//			logdisplay_printf(log, "examining %s", vcmdname);
//			logdisplay_flushline(log);
//		}

		location waitloc = curvcmd->location;
		location curloc;
		train_get_loc(this, &curloc);
		char buf[100];
		vcmd2str(buf, curvcmd);
		ASSERT(!location_isundef(&curloc), "train %d location undef while running %s", this->no, buf);

		// TODO this fucks up
		fixed dist = location_dist_dir(&curloc, &waitloc);

		switch(curvcmd->name) {
	//		case VCMD_SETREVERSE:
	//			traincmdbuffer_put(tid_traincmdbuf, REVERSE, this->no, NULL);
	//			break;
			case VCMD_SETSPEED:
				if (fixed_cmp(dist, fixed_new(40)) < 0) {
					int speed = curvcmd->data.speed;
					train_speed(this->no, speed, tid_traincmdbuf);
					this->vcmdidx++;
					lastvcmd = curvcmd;
					continue;
				}
				break;
//			case VCMD_WAITFORLOC: {
//				location waitloc = curvcmd->location;
//				location curloc;
//				train_get_loc(this, &curloc);
//				char buf[100];
//				vcmd2str(buf, curvcmd);
//				ASSERT(location_isvalid(&curloc), "train %d location invalid while running %s", this->no, buf);
//
//				fixed dist = location_dist_min(&curloc, &waitloc);
//
//				if (fixed_cmp(dist, fixed_new(40)) < 0) {
//					this->vcmdidx++;
//					char buf[100];
//					location2str(buf, &curloc);
//					logdisplay_printf(log, "finished waiting .. curloc:%s, dist:%F", buf, dist);
//					logdisplay_flushline(log);
//					// this is a terrible way to do it. run it once more.
//					train_run_vcmd(this, tid_traincmdbuf, nodemap, log);
//				}
//				break;
//			}
			case VCMD_SETSWITCH: {
//				fixed switch_dist = fixed_div(train_get_stopdist(this), fixed_new(2));
				fixed switch_dist = train_get_stopdist(this);
				if (fixed_cmp(dist, switch_dist) < 0) {
					char *branchname = curvcmd->data.switchinfo.nodename;
					char pos = curvcmd->data.switchinfo.pos;
					track_node *sw = lookup_get(nodemap, branchname);
					ASSERT(sw, "switch is null for %s", branchname);
					char temp[5];
					branchname += strgetw(branchname, temp, 5);
					int branchno = strgetui(&branchname);
					ASSERT(branchno > 0, "wrong branch no %d", branchno);
					this->vcmdidx++;
					train_switch(branchno, pos, tid_traincmdbuf);
					train_solenoidoff(tid_traincmdbuf);
					lastvcmd = curvcmd;
					continue;
				}
				break;
			}
			case VCMD_STOP: {
				fixed stopdist = train_get_stopdist(this);
				if (fixed_cmp(dist, stopdist) < 0) {
					char buf[100], buf2[100];
					location2str(buf, &curloc);
					location2str(buf2, &waitloc);
					logdisplay_printf(log, "curloc:%s", buf);
					logdisplay_flushline(log);
					logdisplay_printf(log, "waitloc:%s", buf2);
					logdisplay_flushline(log);
					logdisplay_printf(log, "dist:%F,stopdist:%F", dist, stopdist);
					logdisplay_flushline(log);
					train_speed(this->no, 0, tid_traincmdbuf);
					this->vcmdidx++;
					lastvcmd = curvcmd;
					continue;
				}
				break;
			}
			default: {
				ASSERT(0, "unknown command %s", vcmdname);
				break; // unreachable
			}
		}
		break;
	}

	if (this->vcmdslen > 0 && this->vcmdidx == this->vcmdslen) {
		this->vcmdidx = 0;
		this->vcmdslen = 0;
		this->destination = location_undef();
	}
}
