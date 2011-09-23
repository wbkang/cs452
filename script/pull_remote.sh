#!/bin/sh

PRIVATEKEY=/c/users/wbkang/.ssh/privatekey

echo "Remote pull"
ssh -i $PRIVATEKEY wbkang@linux024.student.cs.uwaterloo.ca  \
"
source cs452.sh;
cd cs452/kernel;
hg pull;
"	
