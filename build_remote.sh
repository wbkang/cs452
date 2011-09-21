#!/bin/sh

CURCS=`hg log | head -1 | awk '{ print $2 }' | cut -d ":" -f 2`
PRIVATEKEY=/c/users/wbkang/.ssh/privatekey

hg commit -m "temp"

if [ $? -ne 0 ]; then
	echo "Just running remote build only"
	ssh -i $PRIVATEKEY wbkang@linux024.student.cs.uwaterloo.ca  \
	"
source cs452.sh;
cd cs452/kernel;
./clean.sh;
./build.sh;
"
fi
	
	exit 1;
fi

TEMPCS=`hg log | head -1 | awk '{ print $2 }' | cut -d ":" -f 2`

hg push -f -r $TEMPCS 

ssh -i $PRIVATEKEY wbkang@linux024.student.cs.uwaterloo.ca \
"
source cs452.sh;
cd cs452/kernel;
hg update -C $TEMPCS  > /dev/null;
./clean.sh;
./build.sh;
hg update `hg parent | grep changeset: | awk '{print $2}' | cut -d ':' -f 1`
hg strip $TEMPCS > /dev/null;"

hg update $CURCS
hg revert -a -r $TEMPCS
hg strip $TEMPCS