#!/bin/bash
set -e

LIBS=bwio

for i in $LIBS; do
	(cd $i; make clean;);
done

rm lib/*

(cd main; make clean; );

echo cleaned.
