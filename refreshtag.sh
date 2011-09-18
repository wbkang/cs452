#!/bin/sh
ctags  -h .c.h -V  --totals=yes `find . -name '*.[h|c]' -type f`

updatedb -o mlocate.db
