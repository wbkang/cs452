#!/bin/bash
set -e

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

cp -f main/main.elf $ELF && echo "Copy successful"
chmod 776 $ELF

echo BUILT AND DEPLOYED
