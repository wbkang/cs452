#!/bin/bash

LIBS=kernel

mkdir -p lib;

for i in $LIBS; do
	(cd $i;  make -j8; cp $i.a ../lib/lib$i.a);
done

(cd main; make -j8 );

if [ $? -ne 0 ]; then
	echo "Build Failed."
	exit 1
fi

if [ `whoami` = 'wbkang' ]; then
	ELF=/u/cs452/tftp/ARM/sh.elf
else
	ELF=/u/cs452/tftp/ARM/s.elf
fi

cp -f main/main.elf $ELF
chgrp -f cs452_sf $ELF
chmod -f 776 $ELF

echo Deployed
