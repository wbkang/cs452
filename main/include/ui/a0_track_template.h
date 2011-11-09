#pragma once
/* this file is auto-generated from templategen.sh. */
#include<console.h>

#define CONSOLE_LOG_COL 1
#define CONSOLE_LOG_LINE 29
#define CONSOLE_CMD_COL 2
#define CONSOLE_CMD_LINE 30
#define CONSOLE_SENSOR_COL 17
#define CONSOLE_SENSOR_LINE 6

typedef enum { TRACK_A, TRACK_B } track;
	
typedef struct {
	enum direction {
		UNKNOWN, NORTH, SOUTH, EAST, WEST, NORTHEAST, SOUTHEAST, NORTHWEST, SOUTHWEST
	} dir;
	int row, col;
	char *dir_str;
} sensor_pic_info;

typedef struct {
	int row, col;
	char straight, curved;
} switch_pic_info;

void init_track_template(track t, console *c);
sensor_pic_info *get_sensor_pic_info(char mod, int id);
switch_pic_info *get_switch_pic_info(int iswitch);
#define TRACK_TEMPLATE_A CONSOLE_CLEAR "Uptime: 00.00.00s\n" \
"Switches:  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 1" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 2" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 3" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 4" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 5" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 6" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\n" \
"           " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 7" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 8" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 9" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 10" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 11" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 12" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\n" \
"          " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 13" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 14" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 15" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 16" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 17" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 18" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\n" \
"         " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 153" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 154" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 155" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 156" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\n" \
"Recent Sensors: \n" \
"Track:\n" \
"       " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 12" CONSOLE_EFFECT(EFFECT_RESET) "   " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 11" CONSOLE_EFFECT(EFFECT_RESET) "\n" \
"[====" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "======" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "=============" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==============" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==\\\n" \
"    " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 4" CONSOLE_EFFECT(EFFECT_RESET) " //     //         " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 13" CONSOLE_EFFECT(EFFECT_RESET) "        " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 10" CONSOLE_EFFECT(EFFECT_RESET) "           \\\\ \n" \
"[==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "/ " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 14" CONSOLE_EFFECT(EFFECT_RESET) " /" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "=========" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "===" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "=======" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\\" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 9" CONSOLE_EFFECT(EFFECT_RESET) " \n" \
"     //      ||           " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " \\\\" CONSOLE_EFFECT(EFFECT_RESET) "   -- " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " //" CONSOLE_EFFECT(EFFECT_RESET) "             ||\n" \
"[=" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==/       ||            " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " \\\\" CONSOLE_EFFECT(EFFECT_RESET) "  ||" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " //" CONSOLE_EFFECT(EFFECT_RESET) "              ||\n" \
"             " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "()" CONSOLE_EFFECT(EFFECT_RESET) "              \\\\ ||//               ||\n" \
"             ||               \\\\|" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "/" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 155" CONSOLE_EFFECT(EFFECT_RESET) "            ||\n" \
"             ||           " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 156" CONSOLE_EFFECT(EFFECT_RESET) " \\" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "|                 ||\n" \
"             ||                 ||                 ||\n" \
"             ||                 |" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\\" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 154" CONSOLE_EFFECT(EFFECT_RESET) "            ||\n" \
"             ||           " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 153" CONSOLE_EFFECT(EFFECT_RESET) " /" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "|\\\\               ||\n" \
"             " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "()" CONSOLE_EFFECT(EFFECT_RESET) "               //|| \\\\              ||\n" \
"[=" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==\\       ||             " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " //" CONSOLE_EFFECT(EFFECT_RESET) " || " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " \\\\" CONSOLE_EFFECT(EFFECT_RESET) "             ||\n" \
"  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 1" CONSOLE_EFFECT(EFFECT_RESET) " \\\\      ||         " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 16" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " //" CONSOLE_EFFECT(EFFECT_RESET) "  --  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " \\\\" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 17" CONSOLE_EFFECT(EFFECT_RESET) "         ||\n" \
"[=" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "=O=" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\\ " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 15" CONSOLE_EFFECT(EFFECT_RESET) " \\" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==========" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "===" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "======" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "/" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 8" CONSOLE_EFFECT(EFFECT_RESET) " \n" \
"   " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 2" CONSOLE_EFFECT(EFFECT_RESET) "  \\\\     \\\\         " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 6" CONSOLE_EFFECT(EFFECT_RESET) "            " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 7" CONSOLE_EFFECT(EFFECT_RESET) "         //\n" \
"[=" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\\     \\==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "=========" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "====" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "====" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==/\n" \
"      " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 3" CONSOLE_EFFECT(EFFECT_RESET) " \\\\             " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 18" CONSOLE_EFFECT(EFFECT_RESET) " \\\\        //" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 5" CONSOLE_EFFECT(EFFECT_RESET) "\n" \
"[=" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "====" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "===============" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "========" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==========]\n" \
"<result of the last command goes here>\n" \
"$\n" \
""
#define TRACK_TEMPLATE_B CONSOLE_CLEAR "Uptime: 00.00.00s\n" \
"Switches:  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 1" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 2" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 3" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 4" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 5" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 6" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\n" \
"           " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 7" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 8" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 9" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 10" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 11" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 12" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\n" \
"          " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 13" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 14" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 15" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 16" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 17" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) " " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 18" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\n" \
"         " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 153" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 154" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 155" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 156" CONSOLE_EFFECT(EFFECT_RESET) "" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\n" \
"Recent Sensors: \n" \
"Track:\n" \
"             " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 5" CONSOLE_EFFECT(EFFECT_RESET) "       " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 18" CONSOLE_EFFECT(EFFECT_RESET) "                  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 3" CONSOLE_EFFECT(EFFECT_RESET) "\n" \
"[==========" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "========" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "===============" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "====" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "=]\n" \
"           " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 7" CONSOLE_EFFECT(EFFECT_RESET) "//      " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 6" CONSOLE_EFFECT(EFFECT_RESET) "\\\\                 \\\\" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 2" CONSOLE_EFFECT(EFFECT_RESET) "     \n" \
"  /==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "====" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "====" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "=========" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==\\     \\" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "=]\n" \
" //          " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 17" CONSOLE_EFFECT(EFFECT_RESET) "       " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 16" CONSOLE_EFFECT(EFFECT_RESET) "           \\\\     \\\\" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 1" CONSOLE_EFFECT(EFFECT_RESET) "     \n" \
"/" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "======" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "===" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==========" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\\" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 15" CONSOLE_EFFECT(EFFECT_RESET) "  \\" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "=" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "=O=]\n" \
"||" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 8" CONSOLE_EFFECT(EFFECT_RESET) "         " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " \\\\" CONSOLE_EFFECT(EFFECT_RESET) "   -- " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " //" CONSOLE_EFFECT(EFFECT_RESET) "             ||      \\\\     \n" \
"||            " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " \\\\" CONSOLE_EFFECT(EFFECT_RESET) "  ||" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " //" CONSOLE_EFFECT(EFFECT_RESET) "              ||      ||\n" \
"||              \\\\ ||//               " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "()" CONSOLE_EFFECT(EFFECT_RESET) "      " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "()" CONSOLE_EFFECT(EFFECT_RESET) "     \n" \
"||               \\\\|" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "/" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 153" CONSOLE_EFFECT(EFFECT_RESET) "            ||      ||     \n" \
"||           " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 154" CONSOLE_EFFECT(EFFECT_RESET) " \\" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "|                 ||      ||     \n" \
"||                 ||                 ||      ||     \n" \
"||                 |" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "\\" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 156" CONSOLE_EFFECT(EFFECT_RESET) "            ||      ||    \n" \
"||           " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 155" CONSOLE_EFFECT(EFFECT_RESET) " /" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "|\\\\               ||      ||     \n" \
"||               //|| \\\\              " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "()" CONSOLE_EFFECT(EFFECT_RESET) "      " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "()" CONSOLE_EFFECT(EFFECT_RESET) "     \n" \
"||             " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " //" CONSOLE_EFFECT(EFFECT_RESET) " || " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " \\\\" CONSOLE_EFFECT(EFFECT_RESET) "             ||      ||     \n" \
"||" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 9" CONSOLE_EFFECT(EFFECT_RESET) "          " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " //" CONSOLE_EFFECT(EFFECT_RESET) "  --  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) " \\\\" CONSOLE_EFFECT(EFFECT_RESET) "         " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 14" CONSOLE_EFFECT(EFFECT_RESET) "||   " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 4" CONSOLE_EFFECT(EFFECT_RESET) " //     \n" \
"\\" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "=======" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "===" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "=========" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "/     /" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==]\n" \
" \\\\           " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 10" CONSOLE_EFFECT(EFFECT_RESET) "       " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 13" CONSOLE_EFFECT(EFFECT_RESET) "       " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 11" CONSOLE_EFFECT(EFFECT_RESET) "//  " CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_BLUE) " 12" CONSOLE_EFFECT(EFFECT_RESET) "//      \n" \
"  \\==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "=====" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "======================" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "======" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_YELLOW) "?" CONSOLE_EFFECT(EFFECT_RESET) "==" CONSOLE_EFFECT(EFFECT_BRIGHT) "" CONSOLE_EFFECT(EFFECT_FG_WHITE) "O" CONSOLE_EFFECT(EFFECT_RESET) "====]\n" \
"                                              \n" \
"<result of the last command goes here>\n" \
"$\n" \
""