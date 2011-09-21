#!/bin/sh

if ! grep -v "?"; then
	echo "no changes..."
	exit 1
fi

CURCS=`hg log | head -1 | awk '{ print $2 }' | cut -d ":" -f 2`
PRIVATEKEY=/c/users/wbkang/.ssh/privatekey

hg commit -m "temp"

if [ $? -ne 0 ]; then
	echo "FAILED TO COMMIT"
	exit 1;
fi

TEMPCS=`hg log | head -1 | awk '{ print $2 }' | cut -d ":" -f 2`

hg push -f -r $TEMPCS 

ssh -i $PRIVATEKEY wbkang@linux024.student.cs.uwaterloo.ca \
"
source cs452.sh;
cd cs452/kernel;
hg update $TEMPCS  > /dev/null;
./clean.sh;
./build.sh;
hg strip $TEMPCS > /dev/null;"

hg update $CURCS
hg revert -a -r $TEMPCS
hg strip $TEMPCS