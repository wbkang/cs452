#!/bin/bash

LIBS=kernel

mkdir -p lib;

for i in $LIBS; do
	(cd $i;  make; cp $i.a ../lib/lib$i.a);
done

(cd main; make );

if [ $? -ne 0 ]; then
	echo "Build Failed."
	exit 1
fi

ELF=/u/cs452/tftp/ARM/s.elf

cp -f main/main.elf $ELF
chgrp -f cs452_sf $ELF
chmod -f 776 $ELF

echo Deployed
