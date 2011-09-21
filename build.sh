#!/bin/bash
set -x
set -e

LIBS=kernel

mkdir -p lib; 

for i in $LIBS; do
	(cd $i;  make; cp $i.a ../lib/lib$i.a);
done

(cd main; make );

cp main/main.elf /u/cs452/tftp/ARM/space.elf
chmod 770 /u/cs452/tftp/ARM/space.elf

echo BUILT AND DEPLOYED
