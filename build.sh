#!/bin/bash
set -e

LIBS=kernel

mkdir -p lib; 

for i in $LIBS; do
	(cd $i;  make; cp $i.a ../lib/lib$i.a);
done

(cd main; make );

ELF=/u/cs452/tftp/ARM/s.elf

cp main/main.elf $ELF && echo "Copy successful"
chgrp cs452_sf $ELF
chmod 776 $ELF

echo BUILT AND DEPLOYED
