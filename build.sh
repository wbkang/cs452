#!/bin/bash
set -e

LIBS=kernel

for i in $LIBS; do
	(cd $i;  make; cp $i.a ../lib/lib$i.a);
done

(cd main; make );

cp main/main.elf /u/cs452/tftp/ARM/space.elf

echo BUILT AND DEPLOYED
