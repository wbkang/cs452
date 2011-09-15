#!/bin/bash
set -e

LIBS=bwio

for i in $LIBS; do
	(cd $i; make clean; make; cp $i.a ../lib/lib$i.a);
done

(cd main; make clean; make;);

cp main/main.elf /u/cs452/tftp/ARM/wbkang_main.elf

echo BUILT AND DEPLOYED