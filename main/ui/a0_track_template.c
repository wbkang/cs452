#include <ui/a0_track_template.h>
#include <train.h>
#include <util.h>
#include <console.h>


static switch_pic_info switch_pic_info_table_a[] = {
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

static switch_pic_info switch_pic_info_table_b[] = {
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

static switch_pic_info *cur_switch_pic_info_table;

static char direction_str[9][3] = { "", "/\\", "\\/", ">", "<", "/\\", "\\/", "/\\", "\\/" };

static sensor_pic_info sensor_pic_info_table[TRAIN_NUM_MODULES][TRAIN_NUM_SENSORS];


static enum direction opposite_direction[] = {
	UNKNOWN, SOUTH, NORTH, WEST, EAST, SOUTHWEST, NORTHWEST, SOUTHEAST, NORTHEAST
};


static void sensor_pic_def(char mod, int id1, enum direction dir1, int id2, int row, int col) {
	sensor_pic_info_table[mod - 'A'][id1].dir = dir1;
	sensor_pic_info_table[mod - 'A'][id1].row = row;
	sensor_pic_info_table[mod - 'A'][id1].col = col;
	sensor_pic_info_table[mod - 'A'][id1].dir_str = direction_str[dir1];
	sensor_pic_info_table[mod - 'A'][id2].dir = opposite_direction[dir1];
	sensor_pic_info_table[mod - 'A'][id2].row = row;
	sensor_pic_info_table[mod - 'A'][id2].col = col;
	sensor_pic_info_table[mod - 'A'][id2].dir_str = direction_str[opposite_direction[dir1]];
}

static void sensor_pic_def_op(char mod, int id1, enum direction dir1, int id2, int row, int col) {
	sensor_pic_def(mod, id1, opposite_direction[dir1], id2, row, col);
}

static void init_sensor_pic(track t) {
	if (t == TRACK_A) {
		sensor_pic_def('A', 1, EAST, 2, 9, 6);
		sensor_pic_def('C', 13, EAST, 14, 9, 19);
		sensor_pic_def('E', 7, EAST, 8, 9, 33);
		sensor_pic_def('D', 7, EAST, 8, 9, 48);
		sensor_pic_def('A', 13, EAST, 14, 11, 4);
		sensor_pic_def('C', 11, EAST, 12, 11, 18);
		sensor_pic_def('B', 5, EAST, 6, 11, 31);
		sensor_pic_def('D', 3, EAST, 4, 11, 35);
		sensor_pic_def('E', 5, EAST, 6, 11, 41);
		sensor_pic_def('D', 6, EAST, 5, 11, 49);
		sensor_pic_def('E', 15, NORTHWEST, 16, 12, 28);
		sensor_pic_def('E', 4, NORTHEAST, 3, 12, 37);
		sensor_pic_def('A', 16, EAST, 15, 13, 3);
		sensor_pic_def('E', 2, NORTHWEST, 1, 13, 29);
		sensor_pic_def('D', 2, NORTHEAST, 1, 13, 36);
		sensor_pic_def('A', 3, NORTH, 4, 14, 14);
		sensor_pic_def('B', 15, NORTH, 16, 20, 14);
		sensor_pic_def('A', 11, EAST, 12, 21, 2);
		sensor_pic_def('C', 2, NORTHEAST, 1, 21, 30);
		sensor_pic_def('B', 13, NORTHWEST, 14, 21, 37);
		sensor_pic_def('B', 3, NORTHEAST, 4, 22, 29);
		sensor_pic_def('D', 15, NORTHWEST, 16, 22, 38);
		sensor_pic_def('B', 7, EAST, 8, 23, 3);
		sensor_pic_def('A', 10, EAST, 9, 23, 5);
		sensor_pic_def('C', 10, EAST, 9, 23, 18);
		sensor_pic_def('B', 1, EAST, 2, 23, 32);
		sensor_pic_def('D', 14, EAST, 13, 23, 36);
		sensor_pic_def('E', 14, EAST, 13, 23, 42);
		sensor_pic_def('E', 9, EAST, 10, 23, 49);
		sensor_pic_def('B', 11, EAST, 12, 25, 3);
		sensor_pic_def('A', 8, EAST, 7, 25, 6);
		sensor_pic_def('C', 5, EAST, 6, 25, 19);
		sensor_pic_def('C', 15, EAST, 16, 25, 32);
		sensor_pic_def('D', 12, EAST, 11, 25, 37);
		sensor_pic_def('E', 11, EAST, 12, 25, 43);
		sensor_pic_def('D', 10, EAST, 9, 25, 48);
		sensor_pic_def('B', 9, EAST, 10, 27, 3);
		sensor_pic_def('A', 5, EAST, 6, 27, 8);
		sensor_pic_def('C', 7, EAST, 8, 27, 27);
		sensor_pic_def('C', 3, EAST, 4, 27, 42);
	} else if (t == TRACK_B) {
		sensor_pic_def_op('A', 1, EAST, 2, 27, 48);
		sensor_pic_def_op('C', 13, EAST, 14, 27, 35);
		sensor_pic_def_op('E', 7, EAST, 8, 27, 12);
		sensor_pic_def_op('D', 7, EAST, 8, 27, 6);
		sensor_pic_def_op('A', 13, EAST, 14, 25, 50);
		sensor_pic_def_op('C', 11, EAST, 12, 25, 36);
		sensor_pic_def_op('B', 5, EAST, 6, 25, 23);
		sensor_pic_def_op('D', 3, EAST, 4, 25, 19);
		sensor_pic_def_op('E', 5, EAST, 6, 25, 13);
		sensor_pic_def_op('D', 6, EAST, 5, 25, 5);
		sensor_pic_def_op('E', 15, NORTHWEST, 16, 24, 25);
		sensor_pic_def_op('E', 4, NORTHEAST, 3, 24, 16);
		sensor_pic_def('A', 16, SOUTH, 15, 22, 47);
		sensor_pic_def_op('E', 2, NORTHWEST, 1, 23, 24);
		sensor_pic_def_op('D', 2, NORTHEAST, 1, 23, 17);
		sensor_pic_def_op('A', 3, NORTH, 4, 22, 39);
		sensor_pic_def_op('B', 15, NORTH, 16, 16, 39);
		sensor_pic_def('A', 11, NORTH, 12, 16, 47);
		sensor_pic_def_op('C', 2, NORTHEAST, 1, 15, 23);
		sensor_pic_def_op('B', 13, NORTHWEST, 14, 15, 16);
		sensor_pic_def_op('B', 3, NORTHEAST, 4, 14, 24);
		sensor_pic_def_op('D', 15, NORTHWEST, 16, 14, 17);//1
		sensor_pic_def_op('B', 7, EAST, 8, 13, 51);
		sensor_pic_def_op('A', 10, EAST, 9, 13, 49);
		sensor_pic_def_op('C', 10, EAST, 9, 13, 36);
		sensor_pic_def_op('B', 1, EAST, 2, 13, 22);
		sensor_pic_def_op('D', 14, EAST, 13, 13, 18);
		sensor_pic_def_op('E', 14, EAST, 13, 13, 12);
		sensor_pic_def_op('E', 9, EAST, 10, 13, 5);
		sensor_pic_def_op('B', 11, EAST, 12, 11, 51);
		sensor_pic_def_op('A', 8, EAST, 7, 11, 48);
		sensor_pic_def_op('C', 5, EAST, 6, 11, 35);
		sensor_pic_def_op('C', 15, EAST, 16, 11, 22);
		sensor_pic_def_op('D', 12, EAST, 11, 11, 17);
		sensor_pic_def_op('E', 11, EAST, 12, 11, 11);
		sensor_pic_def_op('D', 10, EAST, 9, 11, 6);
		sensor_pic_def_op('B', 9, EAST, 10, 9, 51);
		sensor_pic_def_op('A', 5, EAST, 6, 9, 46);
		sensor_pic_def_op('C', 7, EAST, 8, 9, 27);
		sensor_pic_def_op('C', 3, EAST, 4, 9, 12);
	} else {
		ASSERT(0, "unknown track %d", t);
	}
}

sensor_pic_info *get_sensor_pic_info(char mod, int id) {
	return &sensor_pic_info_table[mod - 'A'][id];
}

switch_pic_info *get_switch_pic_info(int iswitch) {
	return cur_switch_pic_info_table + iswitch;
}

void init_track_template(track t, console *c) {
	for (int i = 0; i < TRAIN_NUM_MODULES; i++) {
		for (int j = 0; j < TRAIN_NUM_SENSORS; j++) {
			sensor_pic_info_table[i][j].dir = UNKNOWN;
		}
	}

	init_sensor_pic(t);

	console_clear(c);
	console_move(c, 1, 1);

	if (t == TRACK_A) {
		cur_switch_pic_info_table = switch_pic_info_table_a;
		console_printf(c, TRACK_TEMPLATE_A);
	} else if (t == TRACK_B) {
		cur_switch_pic_info_table = switch_pic_info_table_b;
		console_printf(c, TRACK_TEMPLATE_B);
	} else {
		ASSERT(0, "unknown track %d", t);
	}

	console_flush(c);
}

