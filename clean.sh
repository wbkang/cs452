#!/bin/bash
set -e

LIBS=kernel

for i in $LIBS; do
	(cd $i; make clean;);
done

rm -f lib/*

(cd main; make clean; );

echo cleaned.
