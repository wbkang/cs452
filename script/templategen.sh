#!/bin/sh

TEMPLATE=../main/a0_track_template.h

echo "#pragma once" > $TEMPLATE
echo "#include<console.h>" >> $TEMPLATE
echo -n "#define TRACK_TEMPLATE CONSOLE_CLEAR " >> $TEMPLATE

BLUE="\" CONSOLE_EFFECT(EFFECT_FG_BLUE) \""
YELLOW="\" CONSOLE_EFFECT(EFFECT_FG_YELLOW) \""
BRIGHT="\" CONSOLE_EFFECT(EFFECT_BRIGHT) \""
WHITE="\" CONSOLE_EFFECT(EFFECT_FG_WHITE) \""
RESET="\" CONSOLE_EFFECT(EFFECT_RESET) \""

sed 's/^/\"/g' ../main/track_template.txt \
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