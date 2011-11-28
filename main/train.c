#include <train.h>
#include <gps.h>
#include <lookup.h>
#include <string.h>
#include <ui/logdisplay.h>
#include <constants.h>

// @TODO: remember the average velocity per edge as opposed to per track (very hard)
int train_init_cal(train_cal *cal, int train_no) {
	switch (train_no) {
		case 37: {
			cal->stopm = fixed_div(fixed_new(27477), fixed_new(10));
			cal->stopb = fixed_div(fixed_new(-55248), fixed_new(100));
			cal->len_pickup = 50;
			cal->dist2nose = 22;
			cal->dist2tail = 120;

			int data[] = {
				0, 1,
				375, 24412,
				1099, 24828,
				4354, 45681,
				5016, 32530,
				5909, 28733,
				10040, 38175,
				10080, 32287,
				10401, 29063,
				12011, 29060,
				10401, 23254,
				10080, 20093,
				15769, 28908,
				18145, 30863,
				22764, 36842,
				599, 42800,
				1099, 51365,
				2185, 31977,
				5016, 40147,
				10040, 56597,
				10080, 44062,
				10426, 36965,
				10426, 31529,
				10263, 27023,
				10263, 23853,
				12846, 26978,
				11845, 22568,
				16089, 28535
			};

			TRAIN_FOREACH_SPEEDIDX(i) {
				int dx = data[i * 2];
				int dt = data[i * 2 + 1];
				cal->v_avg[i] = fixed_div(fixed_new(dx), fixed_new(dt));
			}

			cal->ai_avg10k = fixed_div(fixed_new(16363), fixed_new(10000));
			cal->ad_avg10k = fixed_div(fixed_new(-16363), fixed_new(10000));

			cal->a_m10k = fixed_div(fixed_new(-2448), fixed_new(10000));
			cal->a_b10k = fixed_div(fixed_new(17159), fixed_new(10000));

			return TRUE;
		}
		case 38: {
			cal->stopm = fixed_div(fixed_new(118403), fixed_new(100));
			cal->stopb = fixed_div(fixed_new(-93842), fixed_new(1000));
			cal->len_pickup = 50;
			cal->dist2nose = 25;
			cal->dist2tail = 80;

			int data[] = {
				0, 1,
				445, 103217,
				3213, 38768,
				4630, 32945,
				7050, 36503,
				8661, 35831,
				11691, 40407,
				10639, 31140,
				12611, 32524,
				14656, 33872,
				12611, 26533,
				21271, 40711,
				18230, 32337,
				19619, 32578,
				36037, 58152,
				445, 103217,
				1410, 25227,
				3850, 35460,
				10539, 64302,
				7091, 32959,
				11412, 42560,
				12028, 38705,
				19340, 52800,
				19661, 47915,
				14166, 31185,
				23150, 46491,
				21564, 39870,
				25735, 44219
			};

			TRAIN_FOREACH_SPEEDIDX(i) {
				int dx = data[i * 2];
				int dt = data[i * 2 + 1];
				cal->v_avg[i] = fixed_div(fixed_new(dx), fixed_new(dt));
			}

			cal->ai_avg10k = fixed_div(fixed_new(30577), fixed_new(10000));
			cal->ad_avg10k = fixed_div(fixed_new(-30577), fixed_new(10000));

			cal->a_m10k = fixed_div(fixed_new(41155), fixed_new(10000));
			cal->a_b10k = fixed_div(fixed_new(4520), fixed_new(10000));

			return TRUE;
		}
		case 39: {
			// cal->stopm = fixed_div(fixed_new(118403), fixed_new(100));
			// cal->stopb = fixed_div(fixed_new(-93842), fixed_new(1000));
			cal->len_pickup = 50;
			cal->dist2nose = 24;
			cal->dist2tail = 143;

			// int data[] = {
			// 	0, 1,
			// 	445, 103217,
			// 	3213, 38768,
			// 	4630, 32945,
			// 	7050, 36503,
			// 	8661, 35831,
			// 	11691, 40407,
			// 	10639, 31140,
			// 	12611, 32524,
			// 	14656, 33872,
			// 	12611, 26533,
			// 	21271, 40711,
			// 	18230, 32337,
			// 	19619, 32578,
			// 	36037, 58152,
			// 	445, 103217,
			// 	1410, 25227,
			// 	3850, 35460,
			// 	10539, 64302,
			// 	7091, 32959,
			// 	11412, 42560,
			// 	12028, 38705,
			// 	19340, 52800,
			// 	19661, 47915,
			// 	14166, 31185,
			// 	23150, 46491,
			// 	21564, 39870,
			// 	25735, 44219
			// };

			// TRAIN_FOREACH_SPEEDIDX(i) {
			// 	int dx = data[i * 2];
			// 	int dt = data[i * 2 + 1];
			// 	cal->v_avg[i] = fixed_div(fixed_new(dx), fixed_new(dt));
			// }

			// cal->ai_avg10k = fixed_div(fixed_new(30577), fixed_new(10000));
			// cal->ad_avg10k = fixed_div(fixed_new(-30577), fixed_new(10000));

			// cal->a_m10k = fixed_div(fixed_new(41155), fixed_new(10000));
			// cal->a_b10k = fixed_div(fixed_new(4520), fixed_new(10000));

			return FALSE;
		}
		default:
			return FALSE;
	}
}

void train_init(train_state *this, int no, struct gps *gps) {
	this->calibrated = train_init_cal(&this->cal, no);

	this->no = no;
	train_set_dir(this, TRAIN_UNKNOWN);

	this->v = fixed_new(0);
	this->v_i = fixed_new(0);
	this->v_f = fixed_new(0);

	this->a10k = fixed_new(0);
	this->a_i10k = fixed_new(0);
	this->ma10k = fixed_new(0);

	this->last_speed = 0;
	this->speed = 0;
	train_set_tspeed(this, 0);

	train_set_lost(this);
	train_set_tsim(this, 0);

	this->gps = gps;

	this->vcmdidx = 0;
	this->vcmdwait = 0;

	this->destination = location_undef();

	this->state = TRAIN_GOOD;

	// @TODO: this is a hack for now, we shouldn't initialized uncalibrated trains
	if (this->calibrated) {
		this->reservation = malloc(sizeof(reservation_req));
		this->reservation->len = 0;

		this->vcmds = malloc(sizeof(trainvcmd) * TRAIN_MAX_VCMD);
		this->vcmdslen = 0;

		this->path = malloc(sizeof(path));
		this->path->start = location_undef();
		this->path->end = location_undef();
		this->path->pathlen = 0;
	} else {
		this->reservation = NULL;

		this->vcmds = NULL;
		this->vcmdslen = 0;

		this->path = NULL;
	}
}

fixed train_get_velocity(train_state *this) {
	return this->v;
}

fixed train_get_cruising_velocity(train_state *this) {
	int speed_idx = train_get_speedidx(this);
	return this->cal.v_avg[speed_idx];
}

int train_is_moving(train_state *this) {
	return fixed_sgn(this->v) >= 0;
}

int train_get_stopdist(train_state *this) {
	fixed v = train_get_velocity(this);
	int dist = fixed_int(fixed_add(fixed_mul(this->cal.stopm, v), this->cal.stopb));
	if (dist < 0) return 0;
	return dist;
}

int train_get_speed(train_state *this) {
	return this->speed;
}

int train_get_speedidx(train_state *this) {
	return train_speed2speedidx(this->last_speed, this->speed);
}

// @TODO: the only external thing that changes a train state is the set_speed/reverse commands. we should keep a history of these commands if we want to be able to rewind the simulation.
void train_set_speed(train_state *this, int speed, int t) {
	ASSERT(TRAIN_GOOD_SPEED(speed), "bad speed %d", speed);

	train_update_simulation(this, t);

	this->last_speed = this->speed;
	this->speed = speed;
	train_set_tspeed(this, t);

	this->v_i = train_get_velocity(this);
	this->v_f = train_get_cruising_velocity(this);

	this->a_i10k = this->a10k;
	this->ma10k = train_accel10k(&this->cal, this->v_i, this->v_f);
}

void train_set_frontloc(train_state *this, location *loc_front) {
	this->loc_front = *loc_front;
}

void train_get_frontloc(train_state *this, location *rv_loc_front) {
	*rv_loc_front = this->loc_front;
}

void train_set_pickuploc(train_state *this, location *loc_pickup) {
	this->loc_front = *loc_pickup;
	location_add(&this->loc_front, fixed_new(train_get_pickup2frontdist(this)));
}

void train_get_pickuploc(train_state *this, location *rv_loc_pickup) {
	*rv_loc_pickup = this->loc_front;
	location_add(rv_loc_pickup, fixed_new(-train_get_pickup2frontdist(this)));
}

void train_set_lost(train_state *this) {
	location undef = location_undef();
	train_set_pickuploc(this, &undef);
}

int train_is_lost(train_state *this) {
	location loc_front;
	train_get_frontloc(this, &loc_front);
	return location_isundef(&loc_front);
}

train_direction train_get_dir(train_state *this) {
	return this->dir;
}

void train_set_dir(train_state *this, train_direction dir) {
	this->dir = dir;
}

void train_reverse_dir(train_state *this) {
	switch (train_get_dir(this)) {
		case TRAIN_FORWARD:
			train_set_dir(this, TRAIN_BACKWARD);
			break;
		case TRAIN_BACKWARD:
			train_set_dir(this, TRAIN_FORWARD);
			break;
		case TRAIN_UNKNOWN:
			train_set_dir(this, TRAIN_UNKNOWN);
			break;
	}
}

void train_on_reverse(train_state *this, int t) {
	train_update_simulation(this, t);
	train_reverse_dir(this);
	location loc_front;
	train_get_frontloc(this, &loc_front);
	location_reverse(&loc_front);
	location_add(&loc_front, fixed_new(train_get_length(this)));
	train_set_frontloc(this, &loc_front);
}

int train_get_tspeed(train_state *this) {
	return this->t_speed;
}

void train_set_tspeed(train_state *this, int t_speed) {
	this->t_speed = t_speed;
}

int train_get_tsim(train_state *this) {
	return this->t_sim;
}

void train_set_tsim(train_state *this, int t_sim) {
	this->t_sim = t_sim;
}

int train_get_length(train_state *this) {
	train_cal *cal = &this->cal;
	return cal->dist2nose + cal->len_pickup + cal->dist2tail;
}

int train_get_poserr(train_state *this) {
	return 300;
}

int train_get_pickup2frontdist(train_state *this) {
	switch (train_get_dir(this)) {
		case TRAIN_FORWARD:
			return this->cal.dist2nose;
		case TRAIN_BACKWARD:
			return this->cal.dist2tail;
		case TRAIN_UNKNOWN:
			return 0;
	}
	return 0; // unreachable
}

// simulate the distance the train would travel from t_i to t_f
// assuming t_i <= t_f and that t_i is relatively close to the current time
fixed train_simulate_dx(train_state *this, int t_i, int t_f) {
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
void train_get_pickuploc_hist(train_state *this, int t_i, location *rv_loc) {
	int t_f = train_get_tsim(this);
	ASSERT(t_i <= t_f, "asking for non-historic data");
	fixed dx = train_simulate_dx(this, t_i, t_f);
	train_get_pickuploc(this, rv_loc);
	location_add(rv_loc, fixed_neg(dx));
}

fixed train_accel10k(train_cal *cal, fixed v_i, fixed v_f) {
	fixed a10k = fixed_mul(cal->a_m10k, fixed_sub(v_f, v_i));
	if (fixed_cmp(v_i, v_f) < 0) {
		a10k = fixed_add(a10k, cal->a_b10k);
	} else {
		a10k = fixed_sub(a10k, cal->a_b10k);
	}
	return a10k;
}

// simulation step is 2^TRAIN_SIM_DT_EXP milliseconds
#define TRAIN_SIM_DT_EXP 0
#define TRAIN_SIM_DT (1 << TRAIN_SIM_DT_EXP)

// @TODO: figure this out better
// static void train_update_accel(train_state *this, int t_f) {
// 	fixed t = fixed_new(t_f - this->t_speed);
// 	fixed dv10k = fixed_sub(this->v_f10k, this->v_i10k);
// 	fixed matov = fixed_div(fixed_mul(this->ma10k, t), dv10k);
// 	fixed dif = fixed_sub(fixed_new(1), matov);
// 	fixed da10k = fixed_mul(fixed_mul(fixed_new(6), this->ma10k), fixed_mul(matov, dif));
// 	this->a10k = fixed_add(this->a_i10k, da10k);
// }

// static void train_update_vel(train_state *this, int t_f) {
// 	fixed diff = fixed_sub(this->v10k, this->v_f10k);
// 	if (fixed_cmp(fixed_abs(diff), fixed_new(10)) > 0) {
// 		train_update_accel(this, t_f);
// 		fixed dt = fixed_new(t_f - this->t_sim);
// 		this->v10k = fixed_add(this->v10k, fixed_mul(this->a10k, dt));
// 	} else {
// 		this->v10k = this->v_f10k;
// 		this->a10k = fixed_new(0);
// 	}
// }

// static void train_update_pos(train_state *this, int t_f) {
// 	train_update_vel(this, t_f);
// 	if (!train_is_lost(this) && train_is_moving(this)) {
// 		fixed dt = fixed_new(t_f - this->t_sim);
// 		fixed dx = fixed_div(fixed_mul(this->v10k, dt), fixed_new(10000));
// 		location_add(&this->loc, dx);
// 	}
// 	this->t_sim = t_f;
// }

static void train_update_pos(train_state *this, int t_f) {
	fixed const margin = fixed_div(fixed_new(1), fixed_new(10000));
	if (fixed_cmp(fixed_abs(fixed_sub(this->v, this->v_f)), margin) > 0) {
		fixed dv = fixed_sub(this->v_f, this->v_i);
		fixed t = fixed_new(t_f - this->t_speed);
		fixed atov10k = fixed_div(fixed_mul(this->ma10k, t), dv);
		fixed atov = fixed_div(atov10k, fixed_new(10000));
		fixed atov2 = fixed_mul(atov, atov);
		fixed atov3 = fixed_mul(atov2, atov);
		fixed B = fixed_mul(fixed_new(3), atov2);
		fixed C = fixed_mul(fixed_new(-2), atov3);
		this->v = fixed_add(this->v_i, fixed_mul(dv, fixed_add(B, C)));
	} else {
		this->v = this->v_f;
		this->a10k = fixed_new(0);
	}
	fixed fdt = fixed_new(t_f - this->t_sim);
	if (!train_is_lost(this) && train_is_moving(this)) {
		fixed dx = fixed_mul(this->v, fdt);
		location loc_front;
		train_get_frontloc(this, &loc_front);
		location_add(&loc_front, dx);
		train_set_frontloc(this, &loc_front);
	}
	this->t_sim = t_f;
}

void train_update_simulation(train_state *this, int t_f) {
	int t_i = train_get_tsim(this);
	for (int t = t_i; t <= t_f; t++) {
		train_update_pos(this, t);
	}
	// if ((t_f - t_i) & 1) {
	// 	train_update_pos(this, t_f);
	// }
}

void train_set_dest(train_state *this, location *dest) {
	this->destination = *dest;
	this->vcmdidx = 0;
	this->vcmdslen = 0;
}

#define RESERVE_BEHIND 0
#define RESERVE_AHEAD 200

static int train_update_reservations(train_state *this, logdisplay *log) {
	if (train_is_lost(this)) {
		reservation_free(this->reservation, this->no);
		return FALSE;
	}

	location trainloc;
	train_get_frontloc(this, &trainloc);

	reservation_req req;
	req.len = 0;

	req.edges[req.len++] = trainloc.edge;

	int toprevnode = fixed_int(trainloc.offset);
	int tonextnode = trainloc.edge->dist - toprevnode;

	int ahead = RESERVE_AHEAD + (12 * train_get_stopdist(this)) / 10 - tonextnode;
	if (!track_walk(trainloc.edge->dest, ahead, MAX_PATH, req.edges, &req.len)) return FALSE;

	int behind = RESERVE_BEHIND + train_get_length(this) - toprevnode;
	if (!track_walk(trainloc.edge->src->reverse, behind, MAX_PATH, req.edges, &req.len)) return FALSE;

	if (!reservation_checkpath(&req, this->no)) return FALSE;

	reservation_free(this->reservation, this->no);
	reservation_path(&req, this->no);
	*this->reservation = req;
	return TRUE;
}

void train_ontick(train_state *this, int tid_traincmdbuf, lookup *nodemap, logdisplay *log, int tick) {
	int reserved = train_update_reservations(this, log);

	if (train_is_lost(this)) {
		if (this->vcmdslen > 0) {
			this->vcmdslen = 0;
		}
		if (!location_isundef(&this->destination)) {
			location nowhere = location_undef();
			train_set_dest(this, &nowhere);
		}
		return;
	}

	// if dest and no commands, reroute
	if (!location_isundef(&this->destination) && this->vcmdslen == 0) {
		// char buf[100];
		// location_tostring(&this->destination, buf);

		gps_findpath(this->gps, this, &this->destination, TRAIN_MAX_VCMD, this->vcmds, &this->vcmdslen, log);
		this->last_run_vcmd = NULL;

		// if (this->vcmdslen == 0) {
			// logdisplay_printf(log, "cant find trip plan");
			// logdisplay_flushline(log);
		// }

		// if (this->vcmdslen > 0) {
		// 	for (int i = 0; i < this->vcmdslen; i++) {
		// 		char vcmdname[100];
		// 		vcmd2str(vcmdname, &this->vcmds[i]);
		// 		logdisplay_printf(log, "trip plan %d: %s", i, vcmdname);
		// 		logdisplay_flushline(log);
		// 	}
		// }
		this->vcmdidx = 0;
	}

	switch (this->state) {
		case TRAIN_GOOD: // following a path
			if (reserved) {
				// follow this path
			} else {
				this->state = TRAIN_BAD;
				this->vcmdslen = 0;
				train_speed(this->no, 0, tid_traincmdbuf);
			}
			break;
		case TRAIN_BAD:
			if (reserved) {
				this->state = TRAIN_GOOD; // follow this path
			} else {
				this->vcmdslen = 0; // bad path, reroute
			}
			break;
	}

	while (this->vcmdidx < this->vcmdslen) { // if there are commands to run
		trainvcmd *curvcmd = &this->vcmds[this->vcmdidx];
		char vcmdname[100];
		vcmd2str(vcmdname, curvcmd);

		if (curvcmd != this->last_run_vcmd) {
			logdisplay_printf(log, "[%2d] examining %s", this->vcmdidx, vcmdname);
			logdisplay_flushline(log);
		}
		this->last_run_vcmd = curvcmd;

		location waitloc = curvcmd->location;
		char buf[100];
		vcmd2str(buf, curvcmd);
		if (train_is_lost(this)) {
			logdisplay_printf(log, "[%2d] i lost the location of train %d. cancel the trip.");
			logdisplay_flushline(log);
			location nowhere = location_undef();
			train_set_dest(this, &nowhere);
		}

		location curloc;
		train_get_frontloc(this, &curloc);

		switch (curvcmd->name) {
			case VCMD_SETREVERSE:
				traincmdbuffer_put(tid_traincmdbuf, REVERSE, this->no, NULL);
				this->vcmdidx++;
				continue;
			case VCMD_SETSPEED: {
				int dist = location_isundef(&waitloc) ? 0 : location_dist_min(&curloc, &waitloc);
				if (dist < 0) break;
				if (dist <= 40) {
					int speed = curvcmd->data.speed;
					train_speed(this->no, speed, tid_traincmdbuf);
					this->vcmdidx++;
					continue;
				}
				break;
			}
			case VCMD_SETSWITCH: {
				int dist = location_isundef(&waitloc) ? 0 : location_dist_min(&curloc, &waitloc);
				if (dist < 0) break;
				int switch_dist = train_get_stopdist(this) + train_get_poserr(this); // ok to be safe
				if (dist <= switch_dist) {
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
					continue;
				}
				break;
			}
			case VCMD_STOP: { // @TODO: this is very dangerous. if this command goes through the train instantly assumes that it's at the destination.
				int dist = location_isundef(&waitloc) ? 0 : location_dist_min(&curloc, &waitloc);
				if (dist < 0) {
					// logdisplay_printf(log, "invalid distance");
					// logdisplay_flushline(log);
					break;
				}
				int stopdist = train_get_stopdist(this);
				// if (train_is_moving(this) && (dist <= stopdist) || xx train_get_poserr(this)) {
				if (dist <= stopdist) {
					char buf[100], buf2[100];
					location_tostring(&curloc, buf);
					location_tostring(&waitloc, buf2);
					logdisplay_printf(log, "curloc: %s", buf);
					logdisplay_flushline(log);
					logdisplay_printf(log, "waitloc: %s", buf2);
					logdisplay_flushline(log);
					logdisplay_printf(log, "dist: %dmm, stopdist: %dmm", dist, stopdist);
					logdisplay_flushline(log);
					train_speed(this->no, 0, tid_traincmdbuf);
					this->vcmdidx++;
					continue;
				}
				break;
			}
			case VCMD_WAITFORMS:
				if (this->vcmdwait == 0) {
					this->vcmdwait = tick + MS2TICK(curvcmd->data.timeout);
				}
				if (this->vcmdwait <= tick) {
					logdisplay_printf(log, "done waiting, vcmdwait: %d, t/o: %d, tick: %d", this->vcmdwait, MS2TICK(curvcmd->data.timeout), tick);
					logdisplay_flushline(log);
					this->vcmdwait = 0;
					this->vcmdidx++;
					continue;
				}
				break;
			default: {
				ASSERT(0, "unknown command %s", vcmdname);
				break; // unreachable
			}
		}
		break; // wow this is confusing.. ok
	}

	if (this->vcmdslen > 0 && this->vcmdidx == this->vcmdslen) {
		// location nowhere = location_undef();
		// train_set_dest(this, &nowhere);

		if (fixed_sgn(train_get_velocity(this)) == 0) { // ensure the train stopped
			int time = Time(WhoIs(NAME_TIMESERVER));
			char m = 'A' + (time % 5);
			int id = 1 + (time % 16);
			char name[16];
			sprintf(name, "%c%d", m, id);
			location dest = location_fromnode(lookup_get(nodemap, name), 0);
			train_set_dest(this, &dest);
		}
	}
}

int train_get_reverse_cost(train_state *train, int dist, track_node *node) {
	int reserve_dist = train_get_length(train) + train_get_poserr(train);

	track_edge *edges[MAX_PATH];
	int num_edges = 0;

	if (!track_walk(node->reverse, reserve_dist, MAX_PATH, edges, &num_edges)) {
		return infinity; // not enough room to reverse
	}

	for (int i = 0; i < num_edges; i++) {
		track_edge *edge = edges[i];
		if (!can_occupy(edge, train->no)) return infinity; // reserved by somebody else
		if (edge->src->type == NODE_BRANCH) return infinity; // too close to a branch
	}

	return train_get_stopdist(train);
}
