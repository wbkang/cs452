#include <train.h>
#include <gps.h>
#include <lookup.h>
#include <string.h>
#include <constants.h>
#include <ui/a0ui.h>

int train_init_cal(train_cal *cal, int train_no) {
	switch (train_no) {
		case 21: {
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

			cal->acceltime = poly_new(0, 13662.3, 0, 0, 0, 0);
			cal->deceltime = poly_new(-1.02274, 103842, -695046, 1.09098e6, -1.94579e6, 0);

			return TRUE;
		}
		case 35: {
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

			cal->acceltime = poly_new(0, 9506.3, 0, 0, 0, 0);
			cal->deceltime = poly_new(0, 5835.55, 0, 0, 0, 0);

			return TRUE;
		}
		case 37: {
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

			cal->acceltime = poly_new(16.8551, 10298.2, -6118.3, 0, 0, 0);
			// cal->deceltime = poly_new(0, 5906.36, 0, 0, 0, 0);
			cal->deceltime = poly_new(49.4653, 6227.4, 0, 0, 0, 0);

			return TRUE;
		}
		case 38: {
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

			cal->acceltime = poly_new(-0.00897707, 14008.5, -43208.4, 64574.3, -36555.3, 0);
			cal->deceltime = poly_new(-0.00897707, 14008.5, -43208.4, 64574.3, -36555.3, 0);

			return TRUE;
		}
		case 39: {
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

			cal->acceltime = poly_new(0, 16222.8, -10912.2, 0, 0, 0);
			cal->deceltime = poly_new(-0.0580817, 19572.6, -59037.1, 77626.7, -36317.5, 0);

			return TRUE;
		}
		case 41: {
			cal->len_pickup = 50;
			cal->dist2nose = 24;
			cal->dist2tail = 143;

			int data[] = {
				0,0,
				404,30226,
				485,6212,
				599,4473,
				4743,25616,
				4743,19776,
				4299,14668,
				3967,11197,
				8309,20433,
				4743,1038,
				5031,10073,
				4743,8537,
				4743,7739,
				3967,6408,
				9870,15918,
				689,36048,
				444,5540,
				692,5232,
				1174,6583,
				4144,17400,
				4339,14731,
				3967,11137,
				7667,19069,
				4743,10628,
				5127,1026,
				5342,9501,
				7667,12683,
				5917,9410
			};

			TRAIN_FOREACH_SPEEDIDX(i) {
				float dx = data[i * 2];
				float dt = data[i * 2 + 1];
				cal->v_avg[i] = dx / dt;
			}

			cal->acceltime = poly_new(0, 9525.39, 0, 0, 0, 0);
			// cal->deceltime = poly_new(0, 0, 0, 0, 0, 0);

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

	this->x = 0;

	this->v = 0;
	this->v_i = 0;
	this->v_f = 0;

	this->a = 0;
	this->a_i = 0;
	this->dt = 0;

	this->last_speed = 0;
	this->speed = 0;
	train_set_tspeed(this, 0);

	this->last_attrib_sensor = NULL;
	this->num_missed_sensors = 0;
	this->dist_since_last_sensor = 0;

	train_set_lost(this);
	train_set_tsim(this, 0);

	this->vcmdidx = 0;
	this->vcmdwait = 0;

	this->destination = location_undef();

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
	float dt = train_get_dt(&this->cal, this->v, 0);
	float dist = (this->v / 2) * dt; // + (this->a / 2) * st * st;
	ASSERT(dist >= 0, "stop dist is negative, st: %dms: v: %dmm/s", (int) dt, (int) (this->v * 1000));
	ASSERT(dist < 2000, "stop dist too long, st: %dms: v: %dmm/s", (int) dt, (int) (this->v * 1000));
	return dist;
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

	this->x = 0;

	this->v_i = train_get_velocity(this);
	this->v_f = train_get_cruising_velocity(this);

	this->a_i = this->a;
	// a_f = 0

	if (this->calibrated) {
		this->dt = train_get_dt(&this->cal, this->v_i, this->v_f);
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
	train_set_frontloc(this, &undef);
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
	return 500;
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

void train_on_missed2manysensors(train *this) {
	train_set_lost(this);
	train_speed(this->no, 0, WhoIs(NAME_TRAINCMDBUFFER));
}

// simulate the distance the train would travel from t_i to t_f
// assuming t_i <= t_f and that t_i is relatively close to the current time
float train_simulate_dx(train *this, int t_i, int t_f) {
	if (!train_is_moving(this)) return 0;
	float v = train_get_velocity(this); // @TODO: don't assume current velocity
	float dt = TICK2MS(t_f - t_i);
	return v * dt;
}

float train_get_dt(train_cal *cal, float v_i, float v_f) {
	float dv = fabs(v_f - v_i);
	float dt = poly_eval(v_i < v_f ? &cal->acceltime : &cal->deceltime, dv);
	ASSERT(dt >= -100, "stop time is negative: %d", (int) dt);
	return fmax(0, dt);
}

static void train_update_state(train *this, float t_f) {
	if (fabs(this->v - this->v_f) > 0.001) {
		float dv = this->v_f - this->v_i;
		float dt = this->dt;
		float t = t_f - train_get_tspeed(this);
		float tau = t / dt;
		float tau2 = tau * tau;
		float tau3 = tau * tau2;
		this->v = this->v_i + 3 * dv * tau2 - 2 * dv * tau3;
		this->a = this->a_i + 6 * (dv / dt) * tau * (1 - tau);
	} else {
		this->v = this->v_f;
		this->a = 0;
	}
	float fdt = t_f - train_get_tsim(this);
	if (!train_is_lost(this) && train_is_moving(this)) {
		float dx = this->v * fdt;
		this->dist_since_last_sensor += dx;
		location loc_front = train_get_frontloc(this);
		int num_sensors = location_add(&loc_front, dx);
		if (num_sensors > 0) {
			this->num_missed_sensors += num_sensors;
			if (this->num_missed_sensors >= MAX_NUM_MISSED_SENSORS) {
				train_on_missed2manysensors(this);
				return;
			}
		}
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

void train_on_attrib(train *this, location *new_loc_pickup, int t_loc, int t) {
	track_node *sensor = new_loc_pickup->edge->src;
	ASSERTNOTNULL(sensor); // sanity check
	this->last_attrib_sensor = sensor;
	this->num_missed_sensors = 0;

	train_update_simulation(this, t);

	float dx_lag = train_simulate_dx(this, t_loc, t);

	this->dist_since_last_sensor = dx_lag;

	location_add(new_loc_pickup, dx_lag);
	train_set_pickuploc(this, new_loc_pickup);
}

void train_set_dest(train *this, location *dest) {
	this->destination = *dest;
	this->vcmdidx = 0;
	this->vcmdslen = 0;
}

void train_giveupres(train *this) {
	reservation_req *req = this->reservation_alt;
	req->len = 0;
	reservation_replace(this->reservation, req, this->no);
}

int train_update_reservations(train *this) {
	if (train_is_lost(this)) return FALSE;

	reservation_req *req = this->reservation_alt;
	req->len = 0;

	location loc_train = train_get_frontloc(this);
	req->edges[req->len++] = loc_train.edge;

	int toprevnode = loc_train.offset;
	int tonextnode = loc_train.edge->dist - toprevnode;
	int poserr = train_get_poserr(this);

	int behind = poserr / 2 + train_get_length(this) - toprevnode + this->dist_since_last_sensor;
	if (!track_walk(loc_train.edge->src->reverse, behind, TRACK_NUM_EDGES, req->edges, &req->len)) return FALSE;

	int ahead = poserr / 2 + train_get_stopdist(this) - tonextnode;
	if (!track_walk(loc_train.edge->dest, ahead, TRACK_NUM_EDGES, req->edges, &req->len)) return FALSE;

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

		gps_findpath(gps, this, &this->destination, TRAIN_MAX_VCMD, this->vcmds, &this->vcmdslen, a0ui);
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
				int switch_dist = train_get_stopdist(this) + train_get_poserr(this) / 2; // ok to be safe
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
				// if (train_is_moving(this) && (dist <= stopdist) || train_get_poserr(this)) {
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

	if (this->vcmdslen > 0 && this->vcmdidx == this->vcmdslen) {
		// location nowhere = location_undef();
		// train_set_dest(this, &nowhere);

		if (!train_is_moving(this)) { // ensure the train stopped
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

int train_get_reverse_cost(train *train, int dist, track_node *node) {
	int safe_len = train_get_length(train) + train_get_poserr(train);

	SMALLOC(track_edge*, edges, TRACK_NUM_EDGES);
	int num_edges = 0;

	if (!track_walk(node->reverse, safe_len, TRACK_NUM_EDGES, edges, &num_edges)) {
		return infinity; // not enough room to reverse
	}

	for (int i = 0; i < num_edges; i++) {
		track_edge *edge = edges[i];
		if (!can_occupy(edge, train->no)) return infinity; // reserved by somebody else
		if (edge->src->type == NODE_BRANCH) return infinity; // too close to a branch
	}

	return 2 * safe_len - train_get_length(train);
}
