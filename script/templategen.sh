#!/bin/sh

SCRIPTBASE=`dirname $0`
TEMPLATE=$SCRIPTBASE/../main/include/ui/a0_track_template.h

BLUE="@\" CONSOLE_EFFECT(EFFECT_FG_BLUE) \"@"
YELLOW="@\" CONSOLE_EFFECT(EFFECT_FG_YELLOW) \"@"
BRIGHT="@\" CONSOLE_EFFECT(EFFECT_BRIGHT) \"@"
WHITE="@\" CONSOLE_EFFECT(EFFECT_FG_WHITE) \"@"
RESET="@\" CONSOLE_EFFECT(EFFECT_RESET) \"@"

cat > $TEMPLATE <<EOF
#pragma once
/* this file is auto-generated from templategen.sh. */
#include<console.h>
#include<train.h>
#include<server/uiserver.h>
#include<util.h>

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
	char const *dir_str;
} sensor_pic_info;

typedef struct {
	int row, col;
	char straight, curved;
} switch_pic_info;

typedef struct track_template track_template;

struct track_template {
	ui_id id_ui;
	track track_config;
	switch_pic_info const *switch_pic_info_table;
	sensor_pic_info sensor_pic_info_table[TRAIN_NUM_MODULES][TRAIN_NUM_SENSORS];
	int mod_hist_idx;
	struct {
		char mod; int id;
	} mod_hist[];
};

track_template *track_template_new(track t);
void track_template_updateswitch(track_template *tt, char no, char pos);
void track_template_updatesensor(track_template *tt, char module, int id, int train);
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
