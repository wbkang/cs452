#!/bin/sh


find .  -type f  \( -name '*.c' -o -name '*.h' \) | xargs grep "$1"
