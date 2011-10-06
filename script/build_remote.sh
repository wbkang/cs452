#!/bin/sh

#set -x
cd ..

if [ "$1" = "pbakhila" ]; then
	SILENCER=">/dev/null"
fi

if which hg > /dev/null; then
	HG=hg
elif which /usr/bin/hg; then
	HG=/usr/bin/hg
else
	echo "Mercurial not found!"
	exit 1
fi

REMOTEUSER=$1
REMOTEHOST=linux024.student.cs.uwaterloo.ca
PRIVATEKEY=$2

if [ ! -z "$PRIVATEKEY" ]; then
	echo "Using the specified private key $PRIVATEKEY"
	PKOPT="-i $PRIVATEKEY"
fi

echo "Copying..."

FILES=$($HG status -A | grep -v "^I" | awk '{ print $2 }' | grep -v '\.o' | sed 's/\\/\//g')

tar cjf - $FILES  | ssh $PKOPT $REMOTEUSER@$REMOTEHOST \
"
	[ -f ~/cs452.sh ] && source cs452.sh;
	cd cs452/kernel && \rm -rf *;
	tar xjf -;
	echo "Building..."
	./clean.sh $SILENCER;
	./build.sh $SILENCER; 
"

