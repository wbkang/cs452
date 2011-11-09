#!/bin/sh

SCRIPTBASE=`dirname $0`
TEMPLATE=$SCRIPTBASE/../main/include/ui/a0_track_template.h

BLUE="\" CONSOLE_EFFECT(EFFECT_FG_BLUE) \""
YELLOW="\" CONSOLE_EFFECT(EFFECT_FG_YELLOW) \""
BRIGHT="\" CONSOLE_EFFECT(EFFECT_BRIGHT) \""
WHITE="\" CONSOLE_EFFECT(EFFECT_FG_WHITE) \""
RESET="\" CONSOLE_EFFECT(EFFECT_RESET) \""

cat > $TEMPLATE <<EOF
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
EOF

function generate() {
	symbol=$1
	file=$2
	
	echo -n "#define $symbol CONSOLE_CLEAR " >> $TEMPLATE
	sed 's/^/\"/g' $file \
	| sed 's/\t/    /g' \
	| sed "s/&\([0-9][0-9]*\)/$BRIGHT$BLUE \1$RESET/g" \
	| sed "s/\\?/$BRIGHT$YELLOW\\?$RESET/g" \
	| sed "s/=O=/=$BRIGHT${WHITE}O$RESET=/g" \
	| sed "s/()/$BRIGHT$WHITE()$RESET/g" \
	| sed "s/\^\(\\\\\\\\\)/$BRIGHT$WHITE \\\\\\\\$RESET/g" \
	| sed "s/\^\(\\/\\/\)/$BRIGHT$WHITE \\/\\/$RESET/g" \
	| sed 's/\\/\\\\/g' | sed 's/$/\\n\" \\/g' \
	>> $TEMPLATE
	echo "\"\"" >> $TEMPLATE
}

generate TRACK_TEMPLATE_A $SCRIPTBASE/../main/track_template_a.txt
generate TRACK_TEMPLATE_B $SCRIPTBASE/../main/track_template_b.txt