#!/bin/bash
echo $PATH
ls /usr/local/
HG=/usr/local/bin/hg
CURCS=`$HG log | head -1 | awk '{ print $2 }' | cut -d ":" -f 2`


$HG commit -m "temp"

if [ $? -ne 0 ]; then
	echo "Just running remote build only"
	ssh pbakhila@linux024.student.cs.uwaterloo.ca  \
	"
cd cs452/kernel;
./clean.sh;
./build.sh;
"	
	exit 1;
fi

TEMPCS=`$HG log | head -1 | awk '{ print $2 }' | cut -d ":" -f 2`

$HG push -f -r $TEMPCS 

ssh pbakhila@linux024.student.cs.uwaterloo.ca \
"
source cs452.sh;
cd cs452/kernel;
$HG update -C $TEMPCS  > /dev/null;
./clean.sh;
./build.sh;
$HG update `$HG parent | grep changeset: | awk '{print $2}' | cut -d ':' -f 1`
$HG strip $TEMPCS > /dev/null;"

$HG update $CURCS
$HG revert -a -r $TEMPCS
$HG strip $TEMPCS
