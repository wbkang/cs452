#include <ui/a0_track_template.h>
#include <train.h>
#include <util.h>
#include <console.h>

#define MOD_HIST_LEN 5

static sensor_pic_info *get_sensor_pic_info(track_template *tt, char mod, int id);

static switch_pic_info const switch_pic_info_table_a[] = {
		{ 23, 7, '\\', '=' }, // 1
		{ 25, 9, '\\', '=' }, // 2
		{ 27, 11, '=', '\\' }, // 3
		{ 11, 7, '/', '=' }, // 4
		{ 27, 39, '=', '/' }, // 5
		{ 25, 29, '=', '\\' }, // 6
		{ 25, 40, '=', '/' }, // 7
		{ 23, 52, '/', '=' }, // 8
		{ 11, 52, '\\', '=' }, // 9
		{ 11, 38, '=', '/' }, // 10
		{ 9, 16, '=', '/' }, // 11
		{ 9, 9, '=', '/' }, // 12
		{ 11, 28, '=', '\\' }, // 13
		{ 11, 15, '/', '=' }, // 14
		{ 23, 15, '\\', '=' }, // 15
		{ 23, 29, '=', '/' }, // 16
		{ 23, 39, '=', '\\' }, // 17
		{ 27, 30, '=', '\\' }, // 18
		{ 19, 33, '|', '/' }, // 153
		{ 18, 34, '|', '\\' }, // 154
		{ 15, 34, '|', '/' }, // 155
		{ 16, 33, '|', '\\' }, // 156
};

static switch_pic_info const switch_pic_info_table_b[] = {
		{ 13, 47, '\\', '=' }, // 1
		{ 11, 45, '\\', '=' }, // 2
		{ 9, 43, '=', '\\' }, // 3
		{ 25, 47, '/', '=' }, // 4
		{ 9, 15, '=', '/' }, // 5
		{ 11, 25, '=', '\\' }, // 6
		{ 11, 14, '=', '/' }, // 7
		{ 13, 2, '/', '=' }, // 8
		{ 25, 2, '\\', '=' }, // 9
		{ 25, 16, '=', '/' }, // 10
		{ 27, 38, '=', '/' }, // 11
		{ 27, 45, '=', '/' }, // 12
		{ 25, 26, '=', '\\' }, // 13
		{ 25, 39, '/', '=' }, // 14
		{ 13, 39, '\\', '=' }, // 15
		{ 13, 25, '=', '/' }, // 16
		{ 13, 15, '=', '\\' }, // 17
		{ 9, 24, '=', '\\' }, // 18
		{ 17, 21, '|', '/' }, // 153
		{ 18, 20, '|', '\\' }, // 154
		{ 21, 20, '|', '/' }, // 155
		{ 20, 21, '|', '\\' }, // 156
};

static char const direction_str[9][3] = { "", "/\\", "\\/", ">", "<", "/\\", "\\/", "/\\", "\\/" };

static enum direction opposite_direction[] = {
	UNKNOWN, SOUTH, NORTH, WEST, EAST, SOUTHWEST, NORTHWEST, SOUTHEAST, NORTHEAST
};

static void sensor_pic_def(track_template *tt, char mod, int id1, enum direction dir1, int id2, int row, int col) {
	sensor_pic_info *spinfo1 = &(tt->sensor_pic_info_table[mod - 'A'][id1]);
	sensor_pic_info *spinfo2 = &(tt->sensor_pic_info_table[mod - 'A'][id2]);
	spinfo1->dir = dir1;
	spinfo1->row = row;
	spinfo1->col = col;
	spinfo1->dir_str = direction_str[dir1];
	spinfo2->dir = opposite_direction[dir1];
	spinfo2->row = row;
	spinfo2->col = col;
	spinfo2->dir_str = direction_str[opposite_direction[dir1]];
}

static void sensor_pic_def_op(track_template *tt, char mod, int id1, enum direction dir1, int id2, int row, int col) {
	sensor_pic_def(tt, mod, id1, opposite_direction[dir1], id2, row, col);
}

static void init_sensor_pic(track_template *tt) {
	if (tt->track_config == TRACK_A) {
		sensor_pic_def(tt,'A', 1, EAST, 2, 9, 6);
		sensor_pic_def(tt,'C', 13, EAST, 14, 9, 19);
		sensor_pic_def(tt,'E', 7, EAST, 8, 9, 33);
		sensor_pic_def(tt,'D', 7, EAST, 8, 9, 48);
		sensor_pic_def(tt,'A', 13, EAST, 14, 11, 4);
		sensor_pic_def(tt,'C', 11, EAST, 12, 11, 18);
		sensor_pic_def(tt,'B', 5, EAST, 6, 11, 31);
		sensor_pic_def(tt,'D', 3, EAST, 4, 11, 35);
		sensor_pic_def(tt,'E', 5, EAST, 6, 11, 41);
		sensor_pic_def(tt,'D', 6, EAST, 5, 11, 49);
		sensor_pic_def(tt,'E', 15, NORTHWEST, 16, 12, 28);
		sensor_pic_def(tt,'E', 4, NORTHEAST, 3, 12, 37);
		sensor_pic_def(tt,'A', 16, EAST, 15, 13, 3);
		sensor_pic_def(tt,'E', 2, NORTHWEST, 1, 13, 29);
		sensor_pic_def(tt,'D', 2, NORTHEAST, 1, 13, 36);
		sensor_pic_def(tt,'A', 3, NORTH, 4, 14, 14);
		sensor_pic_def(tt,'B', 15, NORTH, 16, 20, 14);
		sensor_pic_def(tt,'A', 11, EAST, 12, 21, 2);
		sensor_pic_def(tt,'C', 2, NORTHEAST, 1, 21, 30);
		sensor_pic_def(tt,'B', 13, NORTHWEST, 14, 21, 37);
		sensor_pic_def(tt,'B', 3, NORTHEAST, 4, 22, 29);
		sensor_pic_def(tt,'D', 15, NORTHWEST, 16, 22, 38);
		sensor_pic_def(tt,'B', 7, EAST, 8, 23, 3);
		sensor_pic_def(tt,'A', 10, EAST, 9, 23, 5);
		sensor_pic_def(tt,'C', 10, EAST, 9, 23, 18);
		sensor_pic_def(tt,'B', 1, EAST, 2, 23, 32);
		sensor_pic_def(tt,'D', 14, EAST, 13, 23, 36);
		sensor_pic_def(tt,'E', 14, EAST, 13, 23, 42);
		sensor_pic_def(tt,'E', 9, EAST, 10, 23, 49);
		sensor_pic_def(tt,'B', 11, EAST, 12, 25, 3);
		sensor_pic_def(tt,'A', 8, EAST, 7, 25, 6);
		sensor_pic_def(tt,'C', 5, EAST, 6, 25, 19);
		sensor_pic_def(tt,'C', 15, EAST, 16, 25, 32);
		sensor_pic_def(tt,'D', 12, EAST, 11, 25, 37);
		sensor_pic_def(tt,'E', 11, EAST, 12, 25, 43);
		sensor_pic_def(tt,'D', 10, EAST, 9, 25, 48);
		sensor_pic_def(tt,'B', 9, EAST, 10, 27, 3);
		sensor_pic_def(tt,'A', 5, EAST, 6, 27, 8);
		sensor_pic_def(tt,'C', 7, EAST, 8, 27, 27);
		sensor_pic_def(tt,'C', 3, EAST, 4, 27, 42);
	} else if (tt->track_config == TRACK_B) {
		sensor_pic_def_op(tt,'A', 1, EAST, 2, 27, 48);
		sensor_pic_def_op(tt,'C', 13, EAST, 14, 27, 35);
		sensor_pic_def_op(tt,'E', 7, EAST, 8, 27, 12);
		sensor_pic_def_op(tt,'D', 7, EAST, 8, 27, 6);
		sensor_pic_def_op(tt,'A', 13, EAST, 14, 25, 50);
		sensor_pic_def_op(tt,'C', 11, EAST, 12, 25, 36);
		sensor_pic_def_op(tt,'B', 5, EAST, 6, 25, 23);
		sensor_pic_def_op(tt,'D', 3, EAST, 4, 25, 19);
		sensor_pic_def_op(tt,'E', 5, EAST, 6, 25, 13);
		sensor_pic_def_op(tt,'D', 6, EAST, 5, 25, 5);
		sensor_pic_def_op(tt,'E', 15, NORTHWEST, 16, 24, 25);
		sensor_pic_def_op(tt,'E', 4, NORTHEAST, 3, 24, 16);
		sensor_pic_def(tt,'A', 16, SOUTH, 15, 22, 47);
		sensor_pic_def_op(tt,'E', 2, NORTHWEST, 1, 23, 24);
		sensor_pic_def_op(tt,'D', 2, NORTHEAST, 1, 23, 17);
		sensor_pic_def_op(tt,'A', 3, NORTH, 4, 22, 39);
		sensor_pic_def_op(tt,'B', 15, NORTH, 16, 16, 39);
		sensor_pic_def(tt,'A', 11, NORTH, 12, 16, 47);
		sensor_pic_def_op(tt,'C', 2, NORTHEAST, 1, 15, 23);
		sensor_pic_def_op(tt,'B', 13, NORTHWEST, 14, 15, 16);
		sensor_pic_def_op(tt,'B', 3, NORTHEAST, 4, 14, 24);
		sensor_pic_def_op(tt,'D', 15, NORTHWEST, 16, 14, 15);//1
		sensor_pic_def_op(tt,'B', 7, EAST, 8, 13, 51);
		sensor_pic_def_op(tt,'A', 10, EAST, 9, 13, 49);
		sensor_pic_def_op(tt,'C', 10, EAST, 9, 13, 36);
		sensor_pic_def_op(tt,'B', 1, EAST, 2, 13, 22);
		sensor_pic_def_op(tt,'D', 14, EAST, 13, 13, 18);
		sensor_pic_def_op(tt,'E', 14, EAST, 13, 13, 12);
		sensor_pic_def_op(tt,'E', 9, EAST, 10, 13, 5);
		sensor_pic_def_op(tt,'B', 11, EAST, 12, 11, 51);
		sensor_pic_def_op(tt,'A', 8, EAST, 7, 11, 48);
		sensor_pic_def_op(tt,'C', 5, EAST, 6, 11, 35);
		sensor_pic_def_op(tt,'C', 15, EAST, 16, 11, 22);
		sensor_pic_def_op(tt,'D', 12, EAST, 11, 11, 17);
		sensor_pic_def_op(tt,'E', 11, EAST, 12, 11, 11);
		sensor_pic_def_op(tt,'D', 10, EAST, 9, 11, 6);
		sensor_pic_def_op(tt,'B', 9, EAST, 10, 9, 51);
		sensor_pic_def_op(tt,'A', 5, EAST, 6, 9, 46);
		sensor_pic_def_op(tt,'C', 7, EAST, 8, 9, 27);
		sensor_pic_def_op(tt,'C', 3, EAST, 4, 9, 12);
	} else {
		ASSERT(0, "unknown track %d", tt->track_config);
	}
}

static sensor_pic_info *get_sensor_pic_info(track_template *tt, char mod, int id) {
	sensor_pic_info *spinfo = &tt->sensor_pic_info_table[mod - 'A'][id];
	ASSERT(spinfo < 0x3f000000, "omfg. tt:%x, mod:%d(%c), id:%d", tt, mod, mod, id);
	return spinfo;
}

static switch_pic_info const *get_switch_pic_info(track_template *tt, int iswitch) {
	return &tt->switch_pic_info_table[iswitch];
}

track_template *track_template_new(console *con, track t) {
	track_template *tt = malloc(sizeof(*tt) + MOD_HIST_LEN * sizeof(tt->mod_hist[0]));
	tt->con = con;
	tt->track_config = t;
	for (int i = 0; i < MOD_HIST_LEN; i++) {
		tt->mod_hist[i].mod = 0;
		tt->mod_hist[i].id = 0;
	}
	tt->mod_hist_idx = 0;

	for (int i = 0; i < TRAIN_NUM_MODULES; i++) {
		for (int j = 0; j < TRAIN_NUM_SENSORS; j++) {
			tt->sensor_pic_info_table[i][j].dir = UNKNOWN;
		}
	}

	init_sensor_pic(tt);

	console_clear(con);
	console_move(con, 1, 1);

	if (t == TRACK_A) {
		tt->switch_pic_info_table = switch_pic_info_table_a;
		console_printf(con, TRACK_TEMPLATE_A);
	} else if (t == TRACK_B) {
		tt->switch_pic_info_table = switch_pic_info_table_b;
		console_printf(con, TRACK_TEMPLATE_B);
	} else {
		ASSERT(0, "unknown track %d", t);
	}

	console_flush(con);
	return tt;
}

void track_template_updateswitch(track_template *tt, char no, char pos) {
	console *c = tt->con;
	int idx = train_switchno2i(no); // 0 based
	int statusrow = 2 + idx / 6;
	int statuscol = 14 + 5 * (idx % 6);
	char pos_name = train_switchpos_straight(pos) ? 'S' : 'C';

	console_move(c, statusrow, statuscol);
	console_effect(c, EFFECT_BRIGHT);
	console_effect(c, EFFECT_FG_YELLOW);
	console_printf(c, "%c", pos_name);
	console_effect_reset(c);

	switch_pic_info const *swinfo = get_switch_pic_info(tt, idx);

	console_move(c, swinfo->row, swinfo->col);
	console_effect(c, EFFECT_BRIGHT);
	console_effect(c, EFFECT_FG_YELLOW);
	console_printf(c, "%c", (pos_name == 'S') ? swinfo->straight : swinfo->curved);
	console_effect_reset(c);
}

void track_template_updatesensor(track_template *tt, char module, int id, int train) {
	(void)train; // for future use
	console *con = tt->con;

	tt->mod_hist[tt->mod_hist_idx].mod = module;
	tt->mod_hist[tt->mod_hist_idx].id = id;
	tt->mod_hist_idx++;
	tt->mod_hist_idx %= MOD_HIST_LEN;

	for (int i = 0, curidx = tt->mod_hist_idx - 1; i < MOD_HIST_LEN; i++, curidx--) {
		if (curidx < 0) curidx = MOD_HIST_LEN - 1;
		if (tt->mod_hist[curidx].mod == 0) break;
		sensor_pic_info *spinfo = get_sensor_pic_info(tt, tt->mod_hist[curidx].mod, tt->mod_hist[curidx].id);

		if (spinfo->dir != UNKNOWN) {
			console_move(con, spinfo->row, spinfo->col);
			switch (i) {
				case 0:
					console_effect(con, EFFECT_BRIGHT);
				case 1:
					console_effect(con, EFFECT_FG_CYAN);
					break;
				default:
					console_effect(con, EFFECT_BRIGHT);
					console_effect(con, EFFECT_FG_BLUE);
					break;
				}

			console_printf(con, "%s", spinfo->dir_str);
			console_effect_reset(con);
		}
	}
	console_flush(con);
}
