#include <train.h>
#include <gps.h>
#include <lookup.h>
#include <string.h>
#include <constants.h>
#include <ui/a0ui.h>

int train_init_cal(train_cal *cal, int train_no) {
	switch (train_no) {
		case 21: {
			cal->stopm = 1620.85475;
			cal->stopb = 153.7681768;
			cal->len_pickup = 50;
			cal->dist2nose = 22;
			cal->dist2tail = 120;

			int data[] = {
				0,		1,
				410,	57679,
				410,	33976,
				410,	13428,
				3302,	36355,
				1970,	20232,
				410,	3788,
				1141,	9107,
				3651,	24828,
				6123,	35245,
				3977,	19243,
				5923,	26703,
				8348,	31389,
				8470,	26969,
				10113,	28414,
				410,	57679,
				410,	33976,
				410,	13551,
				410,	8516,
				1181,	16967,
				5923,	62009,
				5453,	43420,
				5801,	39413,
				7690,	43076,
				5356,	26508,
				16496,	78202,
				11270,	43373,
				13090,	42845
			};

			TRAIN_FOREACH_SPEEDIDX(i) {
				float dx = data[i * 2];
				float dt = data[i * 2 + 1];
				cal->v_avg[i] = dx / dt;
			}

			cal->usepoly = TRUE;
			cal->x0to12 = poly_new(-3.9501, -0.01, 5.115e-5, -3.907e-9, 1.038e-13, 0);
			cal->v0to12 = poly_derive(cal->x0to12);
			cal->a0to12 = poly_derive(cal->v0to12);

			cal->st_order = 4;
			cal->st[0] = -1.02274;
			cal->st[1] = 103842;
			cal->st[2] = -695046;
			cal->st[3] = 1909800;
			cal->st[4] = -1945780;

			cal->st_mul = 0.8;

			return TRUE;
		}
		case 35: {
			cal->stopm = 2012.356051;
			cal->stopb = -219.3890214;
			cal->len_pickup = 50;
			cal->dist2nose = 22;
			cal->dist2tail = 120;

			int data[] = {
				0,		1,
				599,	57161,
				1820,	50056,
				2201,	33563,
				2448,	25694,
				5143,	37170,
				4158,	22807,
				5687,	29132,
				6967,	29462,
				10113,	30261,
				12451,	30343,
				13290,	27007,
				11724,	19785,
				18098,	27217,
				25942,	39056,
				485,	69964,
				405,	17513,
				2210,	45922,
				1044,	12090,
				3591,	28816,
				5738,	35583,
				6383,	31905,
				10880,	45810,
				12519,	41563,
				11044,	30398,
				23186,	49515,
				7749,	14587,
				12061,	19369
			};

			TRAIN_FOREACH_SPEEDIDX(i) {
				float dx = data[i * 2];
				float dt = data[i * 2 + 1];
				cal->v_avg[i] = dx / dt;
			}

			cal->usepoly = TRUE;
			cal->x0to12 = poly_new(-0.0009, 0.134, -0.0002, 1.329e-7, -2.671e-11, 2.425e-15);
			cal->v0to12 = poly_derive(cal->x0to12);
			cal->a0to12 = poly_derive(cal->v0to12);

			cal->st_order = 3;
			cal->st[0] = -8.5976;
			cal->st[1] = 22379.5;
			cal->st[2] = -57487.1;
			cal->st[3] = 48854.3;

			cal->st_mul = 1;

			return TRUE;
		}
		case 37: {
			cal->stopm = 2747.7;
			cal->stopb = -552.48;
			cal->len_pickup = 50;
			cal->dist2nose = 22;
			cal->dist2tail = 120;

			int data[] = {
				0,		1,
				375,	24412,
				1099,	24828,
				4354,	45681,
				5016,	32530,
				5909,	28733,
				10040,	38175,
				10080,	32287,
				10401,	29063,
				12011,	29060,
				10401,	23254,
				10080,	20093,
				15769,	28908,
				18145,	30863,
				22764,	36842,
				599,	42800,
				1099,	51365,
				2185,	31977,
				5016,	40147,
				10040,	56597,
				10080,	44062,
				10426,	36965,
				10426,	31529,
				10263,	27023,
				10263,	23853,
				12846,	26978,
				11845,	22568,
				16089,	28535
			};

			TRAIN_FOREACH_SPEEDIDX(i) {
				float dx = data[i * 2];
				float dt = data[i * 2 + 1];
				cal->v_avg[i] = dx / dt;
			}

			cal->usepoly = TRUE;
			cal->x0to12 = poly_new(-0.016, 0.0292, -6.921e-5, 9.479e-8, -2.396e-11, 2.53e-15);
			cal->v0to12 = poly_derive(cal->x0to12);
			cal->a0to12 = poly_derive(cal->v0to12);

			cal->st_order = 1;
			cal->st[0] = 49.4653;
			cal->st[1] = 6227.4;

			cal->st_mul = 1;

			return TRUE;
		}
		case 38: {
			cal->stopm = 1184.03;
			cal->stopb = -93.842;
			cal->len_pickup = 50;
			cal->dist2nose = 25;
			cal->dist2tail = 80;

			int data[] = {
				0,		1,
				445,	103217,
				3213,	38768,
				4630,	32945,
				7050,	36503,
				8661,	35831,
				11691,	40407,
				10639,	31140,
				12611,	32524,
				14656,	33872,
				12611,	26533,
				21271,	40711,
				18230,	32337,
				19619,	32578,
				36037,	58152,
				445,	103217,
				1410,	25227,
				3850,	35460,
				10539,	64302,
				7091,	32959,
				11412,	42560,
				12028,	38705,
				19340,	52800,
				19661,	47915,
				14166,	31185,
				23150,	46491,
				21564,	39870,
				25735,	44219
			};

			TRAIN_FOREACH_SPEEDIDX(i) {
				float dx = data[i * 2];
				float dt = data[i * 2 + 1];
				cal->v_avg[i] = dx / dt;
			}

			cal->usepoly = FALSE;

			cal->st_order = 4;
			cal->st[0] = -0.00897707;
			cal->st[1] = 14008.5;
			cal->st[2] = -43208.4;
			cal->st[3] = 64574.3;
			cal->st[4] = -36555.3;

			cal->st_mul = 1.0;

			return TRUE;
		}
		case 39: {
			cal->stopm = 1386.5;
			cal->stopb = -68.665;
			cal->len_pickup = 50;
			cal->dist2nose = 24;
			cal->dist2tail = 143;

			int data[] = {
				0,		1,
				791,	78393,
				2510,	34398,
				4757,	37633,
				6982,	39301,
				8734,	37224,
				6509,	22603,
				7694,	22102,
				16481,	42221,
				15437,	35441,
				11279,	23860,
				15062,	28433,
				10558,	18291,
				39277,	63136,
				16803,	27068,
				599,	60079,
				1466,	20227,
				3676,	28987,
				13885,	78019,
				6522,	27782,
				16481,	57568,
				18332,	52594,
				8135,	21220,
				12515,	28530,
				19804,	42189,
				14271,	27137,
				20784,	36490,
				36677,	59104
			};

			TRAIN_FOREACH_SPEEDIDX(i) {
				float dx = data[i * 2];
				float dt = data[i * 2 + 1];
				cal->v_avg[i] = dx / dt;
			}

			cal->usepoly = TRUE;
			cal->x0to12 = poly_new(0.2876, 0.0306, -7.537e-5, 1.334e-7, -7.897e-11, 2.128e-14);
			cal->v0to12 = poly_derive(cal->x0to12);
			cal->a0to12 = poly_derive(cal->v0to12);

			cal->st_order = 4;
			cal->st[0] = -0.0580817;
			cal->st[1] = 19572.6;
			cal->st[2] = -59037.1;
			cal->st[3] = 77626.7;
			cal->st[4] = -36317.5;

			cal->st_mul = 2.2;

			return TRUE;
		}
		case 41: {
			// cal->stopm = 1386.5;
			// cal->stopb = -68.665;
			cal->len_pickup = 50;
			cal->dist2nose = 24;
			cal->dist2tail = 143;

			// int data[] = {
			// 	0,		1,
			// 	791,	78393,
			// 	2510,	34398,
			// 	4757,	37633,
			// 	6982,	39301,
			// 	8734,	37224,
			// 	6509,	22603,
			// 	7694,	22102,
			// 	16481,	42221,
			// 	15437,	35441,
			// 	11279,	23860,
			// 	15062,	28433,
			// 	10558,	18291,
			// 	39277,	63136,
			// 	16803,	27068,
			// 	599,	60079,
			// 	1466,	20227,
			// 	3676,	28987,
			// 	13885,	78019,
			// 	6522,	27782,
			// 	16481,	57568,
			// 	18332,	52594,
			// 	8135,	21220,
			// 	12515,	28530,
			// 	19804,	42189,
			// 	14271,	27137,
			// 	20784,	36490,
			// 	36677,	59104
			// };

			// TRAIN_FOREACH_SPEEDIDX(i) {
			// 	float dx = data[i * 2];
			// 	float dt = data[i * 2 + 1];
			// 	cal->v_avg[i] = dx / dt;
			// }

			cal->usepoly = TRUE;
			cal->x0to12 = poly_new(-1.281, 0.101, -0.0001, 8.216e-8, -1.447e-11, 1.084e-15);
			cal->v0to12 = poly_derive(cal->x0to12);
			cal->a0to12 = poly_derive(cal->v0to12);

			// cal->st_order = 4;
			// cal->st[0] = -0.0580817;
			// cal->st[1] = 19572.6;
			// cal->st[2] = -59037.1;
			// cal->st[3] = 77626.7;
			// cal->st[4] = -36317.5;

			// cal->st_mul = 2.2;

			return FALSE;
		}
		default:
			return FALSE;
	}
}

int train_init(train *this, int no) {
	this->calibrated = train_init_cal(&this->cal, no);

	this->no = no;
	train_set_dir(this, TRAIN_UNKNOWN);

	this->v = 0;
	this->v_i = 0;
	this->v_f = 0;

	this->a = 0;
	this->a_i = 0;
	this->st = 0;

	this->last_speed = 0;
	this->speed = 0;
	train_set_tspeed(this, 0);

	train_set_lost(this);
	train_set_tsim(this, 0);

	this->vcmdidx = 0;
	this->vcmdwait = 0;

	this->destination = location_undef();
	this->stopatdest = TRUE;
	this->reversible = TRUE;

	this->state = TRAIN_GOOD;

	if (this->calibrated) {
		this->reservation = malloc(sizeof(reservation_req));
		this->reservation->len = 0;

		this->reservation_alt = malloc(sizeof(reservation_req));
		this->reservation_alt->len = 0;

		this->vcmds = malloc(sizeof(trainvcmd) * TRAIN_MAX_VCMD);
		this->vcmdslen = 0;

		this->path = malloc(sizeof(path));
		this->path->start = location_undef();
		this->path->end = location_undef();
		this->path->pathlen = 0;
	}

	return this->calibrated;
}

float train_get_velocity(train *this) {
	return this->v;
}

float train_get_cruising_velocity(train *this) {
	return this->cal.v_avg[train_get_speedidx(this)];
}

int train_is_moving(train *this) {
	return train_get_velocity(this) > 0;
}

int train_get_stopdist(train *this) {
	// if (this->v == this->v_f) {
	// 	float v = train_get_velocity(this);
	// 	int dist = this->cal.stopm * v + this->cal.stopb;
	// 	return max(0, dist);
	// }
	float st = train_st(&this->cal, this->v, 0);
	return (this->v / 2) * st; // + (this->a / 2) * st * st;
}

int train_get_speed(train *this) {
	return this->speed;
}

int train_get_speedidx(train *this) {
	return train_speed2speedidx(this->last_speed, train_get_speed(this));
}

void train_set_speed(train *this, int speed, int t) {
	ASSERT(TRAIN_GOOD_SPEED(speed), "bad speed %d", speed);

	train_update_simulation(this, t);

	this->last_speed = this->speed;
	this->speed = speed;
	train_set_tspeed(this, t);

	this->v_i = train_get_velocity(this);
	this->v_f = train_get_cruising_velocity(this);

	this->a_i = this->a;
	// a_f = 0

	if (this->calibrated) {
		this->st = train_st(&this->cal, this->v_i, this->v_f);
	}
}

void train_set_frontloc(train *this, location *loc_front) {
	this->loc_front = *loc_front;
}

location train_get_frontloc(train *this) {
	return this->loc_front;
}

void train_set_pickuploc(train *this, location *loc_pickup) {
	location new_loc_front = *loc_pickup;
	location_add(&new_loc_front, train_get_pickup2frontdist(this));
	train_set_frontloc(this, &new_loc_front);
}

location train_get_pickuploc(train *this) {
	location rv = train_get_frontloc(this);
	location_add(&rv, -train_get_pickup2frontdist(this));
	return rv;
}

// limiting the scope of this function makes it MUCH easier to code. currently this function is used for sensor attribution, to see where the train was at the estimated time of the sensor hit.
// the assumption is that the switches along the path of the train did not change configuration during t_i and t_sim
// @TODO: make less assumptions about the state of the track, perhaps store the locations generated by every train_update_simulation and linearly interpolate between the closes two locations to the given t_i
location train_get_pickuploc_hist(train *this, int t_i) {
	int t_f = train_get_tsim(this);
	ASSERT(t_i <= t_f, "asking for non-historic data");
	location rv = train_get_pickuploc(this);
	float dx = train_simulate_dx(this, t_i, t_f);
	location_add(&rv, -dx);
	return rv;
}

void train_set_lost(train *this) {
	location undef = location_undef();
	train_set_pickuploc(this, &undef);
}

int train_is_lost(train *this) {
	location loc_front = train_get_frontloc(this);
	return location_isundef(&loc_front);
}

train_direction train_get_dir(train *this) {
	return this->dir;
}

void train_set_dir(train *this, train_direction dir) {
	this->dir = dir;
}

void train_on_reverse(train *this, int t) {
	train_update_simulation(this, t);
	switch (train_get_dir(this)) {
		case TRAIN_FORWARD:
			train_set_dir(this, TRAIN_BACKWARD);
			break;
		case TRAIN_BACKWARD:
			train_set_dir(this, TRAIN_FORWARD);
			break;
		case TRAIN_UNKNOWN:
			break;
	}
	location loc_front = train_get_frontloc(this);
	location_reverse(&loc_front);
	location_add(&loc_front, train_get_length(this));
	train_set_frontloc(this, &loc_front);
}

int train_get_tspeed(train *this) {
	return this->t_speed;
}

void train_set_tspeed(train *this, int t_speed) {
	this->t_speed = t_speed;
}

float train_get_tsim(train *this) {
	return this->t_sim;
}

void train_set_tsim(train *this, float t_sim) {
	this->t_sim = t_sim;
}

int train_get_length(train *this) {
	train_cal *cal = &this->cal;
	return cal->dist2nose + cal->len_pickup + cal->dist2tail;
}

// @TODO: this should be a function of the distance travelled since last sensor hit
int train_get_poserr(train *this) {
	return 300;
}

int train_get_pickup2frontdist(train *this) {
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
float train_simulate_dx(train *this, int t_i, int t_f) {
	if (!train_is_moving(this)) return 0;
	float v = train_get_velocity(this); // @TODO: don't assume current velocity
	float dt = TICK2MS(t_f - t_i);
	return v * dt;
}

float train_st(train_cal *cal, float v_i, float v_f) {
	float rv = 0;

	float dv = fabs(v_f - v_i);
	float dvn = 1; // dv^n

	for (int n = 0; n <= cal->st_order; n++) {
		rv += cal->st[n] * dvn;
		dvn *= dv;
	}

	if (v_i < v_f) { // accelerating
		rv *= cal->st_mul;
	}

	return rv;
}

static void train_update_state(train *this, float t_f) {
	if (fabs(this->v - this->v_f) > 0.0001) {
		if (this->v_i < this->v_f && this->cal.usepoly) {
			float dv = this->v_f - this->v_i;
			float alpha = dv / this->cal.v_avg[train_speed2speedidx(0, 12)];
			poly v = poly_scale(this->cal.v0to12, alpha);
			float t = t_f - train_get_tspeed(this);
			this->v = this->v_i + poly_eval(&v, t);
			poly a = poly_derive(v);
			this->a = this->a_i + poly_eval(&a, t);
		} else {
			float dv = this->v_f - this->v_i;
			float dt = this->st;
			float t = t_f - train_get_tspeed(this);
			float tau = t / dt;
			float tau2 = tau * tau;
			// float tau3 = tau * tau2;
			// float tau4 = tau * tau3;

			// a_i=a_f=0
			this->v = this->v_i + dv * (3 - 2 * tau) * tau2;
			this->a = this->a_i + 6 * (dv / dt) * tau * (1 - tau);

			// j_i=j_f=0
			// this->v = this->v_i + dv * (10 - 15 * tau + 6 * tau2) * tau3;
			// this->a = this->a_i + 30 * (dv / dt) * fpow(tau * (1 - tau), 2);

			// s_i=s_f=0
			// this->v = this->v_i + dv * (35 - 84 * tau + 70 * tau2 - 20 * tau3) * tau4;
			// this->a = this->a_i + 140 * (dv / dt) * fpow(tau * (1 - tau), 3);
		}
	} else {
		this->v = this->v_f;
		this->a = 0;
	}
	float fdt = t_f - train_get_tsim(this);
	if (!train_is_lost(this) && train_is_moving(this)) {
		float dx = this->v * fdt;
		location loc_front = train_get_frontloc(this);
		location_add(&loc_front, dx);
		train_set_frontloc(this, &loc_front);
	}
	train_set_tsim(this, t_f);
}

void train_update_simulation(train *this, int t_f) {
	float t_i = train_get_tsim(this);
	for (float t = t_i; t <= t_f; t += 0.5) {
		train_update_state(this, t);
	}
}

location train_get_dest(train *this) {
	return this->destination;
}

void train_set_dest(train *this, location *dest) {
	this->destination = *dest;
	this->vcmdidx = 0;
	this->vcmdslen = 0;
}

#define RESERVE_BEHIND 0
#define RESERVE_AHEAD 100

static int train_update_reservations(train *this) {
	reservation_req *req = this->reservation_alt;
	req->len = 0;

	if (train_is_lost(this)) {
		reservation_replace(this->reservation, req, this->no);
		return FALSE;
	}

	location loc_train = train_get_frontloc(this);

	req->edges[req->len++] = loc_train.edge;

	int toprevnode = loc_train.offset;
	int behind = RESERVE_BEHIND + train_get_length(this) - toprevnode;

	int tonextnode = loc_train.edge->dist - toprevnode;
	int ahead = RESERVE_AHEAD + train_get_stopdist(this) - tonextnode;

	if (!track_walk(loc_train.edge->dest, ahead, TRACK_MAX, req->edges, &req->len)) return FALSE;

	if (!track_walk(loc_train.edge->src->reverse, behind, TRACK_MAX, req->edges, &req->len)) return FALSE;

	return reservation_replace(this->reservation, req, this->no);
}

void train_ontick(train *this, int tid_traincmdbuf, lookup *nodemap, a0ui *a0ui, int tick, struct gps *gps) {
	int reserved = train_update_reservations(this);

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

		gps_findpath(gps, this, &this->destination, TRAIN_MAX_VCMD, this->vcmds, &this->vcmdslen);
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
			a0ui_on_trip_logf(a0ui, "[%2d] examining %s", this->vcmdidx, vcmdname);
		}
		this->last_run_vcmd = curvcmd;

		location waitloc = curvcmd->location;
		char buf[100];
		vcmd2str(buf, curvcmd);
		if (train_is_lost(this)) {
			a0ui_on_trip_logf(a0ui, "[%2d] i lost the location of train %d. cancel the trip.");
			location nowhere = location_undef();
			train_set_dest(this, &nowhere);
		}

		location curloc = train_get_frontloc(this);

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
					a0ui_on_trip_logf(a0ui, "curloc: %s", buf);
					a0ui_on_trip_logf(a0ui, "waitloc: %s", buf2);
					a0ui_on_trip_logf(a0ui, "dist: %dmm, stopdist: %dmm", dist, stopdist);
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
					a0ui_on_trip_logf(a0ui, "done waiting, vcmdwait: %d, t/o: %d, tick: %d", this->vcmdwait, MS2TICK(curvcmd->data.timeout), tick);
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

	location trainloc = train_get_frontloc(this);
	int dist2dest = location_dist_min(&trainloc, &this->destination);

	if (this->vcmdslen > 0 && this->vcmdidx == this->vcmdslen && dist2dest < 40) {
		 location nowhere = location_undef();
		 train_set_dest(this, &nowhere);

		 // this is the random dest code
//		if (!train_is_moving(this)) { // ensure the train stopped
//			int time = Time(WhoIs(NAME_TIMESERVER));
//			char m = 'A' + (time % 5);
//			int id = 1 + (time % 16);
//			char name[16];
//			sprintf(name, "%c%d", m, id);
//			location dest = location_fromnode(lookup_get(nodemap, name), 0);
//			train_set_dest(this, &dest);
//		}
	}
}

int train_get_reverse_cost(train *train, int dist, track_node *node) {
	int safe_len = train_get_length(train) + train_get_poserr(train);

	SMALLOC(track_edge*, edges, TRACK_MAX);
	int num_edges = 0;

	if (!track_walk(node->reverse, safe_len, TRACK_MAX, edges, &num_edges)) {
		return infinity; // not enough room to reverse
	}

	for (int i = 0; i < num_edges; i++) {
		track_edge *edge = edges[i];
		if (!can_occupy(edge, train->no)) return infinity; // reserved by somebody else
		if (edge->src->type == NODE_BRANCH) return infinity; // too close to a branch
	}

	return 2 * safe_len - train_get_length(train);
}
