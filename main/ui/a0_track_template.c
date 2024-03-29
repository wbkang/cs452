#include <ui/a0_track_template.h>
#include <train.h>
#include <util.h>
#include <console.h>
#include <string.h>

#define MOD_HIST_LEN 3

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
	sensor_pic_info *spinfo1 = &(tt->sensor_pic_info_table[mod - 'A'][id1 - 1]);
	sensor_pic_info *spinfo2 = &(tt->sensor_pic_info_table[mod - 'A'][id2 - 1]);
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
	sensor_pic_info *spinfo = &tt->sensor_pic_info_table[mod - 'A'][id - 1];
	ASSERT((uint) spinfo < 0x3f000000, "omfg. tt:%x, mod:%d(%c), id:%d", tt, mod, mod, id);
	return spinfo;
}

static switch_pic_info const *get_switch_pic_info(track_template *tt, int iswitch) {
	return &tt->switch_pic_info_table[iswitch];
}

static int next_token(char const **picture, char *rv_buf) {
	char const *cur = *picture;
	for(;;) {
		if (*cur == '\0') {
			int len = *picture - cur + 1;
			strncpy(rv_buf, *picture, len);
			*picture = cur;
			return FALSE;
		} else if (cur == *picture) { // start of the segment
			if (*cur == '@') {
				char const *effectstart = cur + 1;
				char const *effectend = effectstart + 1;
				while (*effectend != '@') effectend++;
				int len = effectend - effectstart;
				strncpy(rv_buf, effectstart, len);
				rv_buf[len] = '\0';
				*picture = effectend + 1;
				return TRUE;
			} else if (*cur == '\n') {
				rv_buf[0] = '\n';
				rv_buf[1] = '\0';
				*picture += 1;
				return TRUE;
			}
		}
		else if (*cur == '@' || *cur == '\n') { // end of the current segment
			ASSERT(cur != *picture, "WTF?");
			int len = cur - *picture;
			strncpy(rv_buf, *picture, len);
			rv_buf[len] = '\0';
			*picture = cur;
			return TRUE;
		}

		cur++;
	}

	ASSERT(0, "unreachable");
	return 0;
}

static void print_track_picture(char const *picture) {
	char buf[1024];
	int more;
	int effect = 0;
	int color = 0;
	int line = 1;

	ui_id id_ui = uiserver_register();
	uiserver_move(id_ui, 1, 1);

	do {
		more = next_token(&picture, buf);
//		uiserver_printf(tt->id_ui, "[30%s]", buf);

		if (strcmp(buf, CONSOLE_EFFECT(EFFECT_FG_BLUE)) == 0) {
			effect |= UIEFFECT_FGCOLOR;
			color = EFFECT_FG_BLUE;
//			uiserver_printf(tt->id_ui, "[blue]");
		} else if (strcmp(buf, CONSOLE_EFFECT(EFFECT_FG_YELLOW)) == 0) {
			effect |= UIEFFECT_FGCOLOR;
			color = EFFECT_FG_YELLOW;
//			uiserver_printf(tt->id_ui, "[yellow]");
		} else if (strcmp(buf, CONSOLE_EFFECT(EFFECT_FG_WHITE)) == 0) {
			effect |= UIEFFECT_FGCOLOR;
			color = EFFECT_FG_WHITE;
//			uiserver_printf(tt->id_ui, "[white]");
		} else if (strcmp(buf, CONSOLE_EFFECT(EFFECT_BRIGHT)) == 0) {
			effect |= EFFECT_BRIGHT;
//			uiserver_printf(tt->id_ui, "[bright]");
		} else if (strcmp(buf, CONSOLE_EFFECT(EFFECT_RESET)) == 0) {
			effect = 0;
			color = 0;
//			uiserver_printf(tt->id_ui, "[reset]");
		} else if (strcmp(buf, "\n") == 0) {
			line++;
			uiserver_move(id_ui, line, 1);
//			uiserver_printf(tt->id_ui, "[newline]");
		} else {
			uiserver_effect(id_ui, effect, color);
			uiserver_printf(id_ui, buf);
		}
	} while(more);
}

track_template *track_template_new(char tr) {
	track_template *tt = malloc(sizeof(*tt) + MAX_TRAINCOUNT * MOD_HIST_LEN * sizeof(tt->mod_hist[0]));
	track t = (tr == 'a') ? TRACK_A : TRACK_B;
	tt->id_ui = uiserver_register();
	tt->track_config = t;
	for (int train = 0; train < 5; train++) {
		tt->mod_hist_idx[train] = 0;
		for (int i = 0; i < MOD_HIST_LEN; i++) {
			tt->mod_hist[train][i].mod = 0;
			tt->mod_hist[train][i].id = 0;
		}
	}

	for (int i = 0; i < TRAIN_NUM_MODULES; i++) {
		for (int j = 0; j < TRAIN_NUM_SENSORS; j++) {
			tt->sensor_pic_info_table[i][j].dir = UNKNOWN;
		}
	}

	init_sensor_pic(tt);

	uiserver_force_refresh(tt->id_ui);
	console *con = console_new(COM2);

	uiserver_move(tt->id_ui, 1, 1);

	if (t == TRACK_A) {
		tt->switch_pic_info_table = switch_pic_info_table_a;
		print_track_picture(TRACK_TEMPLATE_A);
	} else if (t == TRACK_B) {
		tt->switch_pic_info_table = switch_pic_info_table_b;
		print_track_picture(TRACK_TEMPLATE_B);
	} else {
		ASSERT(0, "unknown track %d", t);
	}

	console_flush(con);
	return tt;
}

void track_template_updateswitch(track_template *tt, char no, char pos) {
	int idx = train_switchno2i(no); // 0 based
	int statusrow = 2 + idx / 6;
	int statuscol = 14 + 5 * (idx % 6);
	char pos_name = track_switchpos_straight(pos) ? 'S' : 'C';
	uiserver_effect(tt->id_ui, UIEFFECT_BRIGHT | UIEFFECT_FGCOLOR, EFFECT_FG_YELLOW);

	uiserver_move(tt->id_ui, statusrow, statuscol);
	uiserver_printf(tt->id_ui, "%c", pos_name);

	switch_pic_info const *swinfo = get_switch_pic_info(tt, idx);

	uiserver_move(tt->id_ui, swinfo->row, swinfo->col);
	uiserver_printf(tt->id_ui, "%c", (pos_name == 'S') ? swinfo->straight : swinfo->curved);
	uiserver_effect(tt->id_ui, 0, 0);
}



void track_template_updatesensor(track_template *tt, char module, int id, int trainidx) {
	const int base_color[5] = { EFFECT_FG_CYAN, EFFECT_FG_MAGENTA, EFFECT_FG_GREEN, EFFECT_FG_RED, EFFECT_FG_YELLOW };
	trainidx %= 5;

	int *mod_hist_idx = &tt->mod_hist_idx[trainidx];
	struct mod_hist_info *mod_hist_infos = tt->mod_hist[trainidx];
	mod_hist_infos[*mod_hist_idx].mod = module;
	mod_hist_infos[*mod_hist_idx].id = id;
	(*mod_hist_idx)++;
	*mod_hist_idx %= MOD_HIST_LEN;

	for (int i = 0, curidx = *mod_hist_idx - 1; i < MOD_HIST_LEN; i++, curidx--) {
		if (curidx < 0) curidx = MOD_HIST_LEN - 1;
		if (mod_hist_infos[curidx].mod == 0) break;
		sensor_pic_info *spinfo = get_sensor_pic_info(tt, mod_hist_infos[curidx].mod, mod_hist_infos[curidx].id);

		if (spinfo->dir != UNKNOWN) {
			uiserver_move(tt->id_ui, spinfo->row, spinfo->col);
			int effect = 0;
			int color = 0;
			switch (i) {
				case 0:
					effect |= UIEFFECT_BRIGHT;
				case 1:
					effect |= UIEFFECT_FGCOLOR;
					color = base_color[trainidx];
					break;
				default:
					effect |= UIEFFECT_FGCOLOR;
					color = EFFECT_FG_BLUE;
					break;
				}

			uiserver_effect(tt->id_ui, effect, color);
			uiserver_printf(tt->id_ui, "%s", spinfo->dir_str);
			uiserver_effect(tt->id_ui, 0, 0);
		}
	}
}
