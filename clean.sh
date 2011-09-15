#!/bin/bash
set -e

LIBS=realtime

for i in $LIBS; do
	(cd $i; make clean;);
done

rm lib/*

(cd main; make clean; );

echo cleaned.
