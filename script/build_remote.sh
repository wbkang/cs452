#!/bin/sh

if which hg; then
	HG=hg
elif which /usr/bin/hg; then
	HG=/usr/bin/hg
else 
	echo "Mercurial not found!"
	exit 1
fi

CURCS=`$HG log | head -1 | awk '{ print $2 }' | cut -d ":" -f 2`
echo "Current changeset: $CURCS"

REMOTEUSER=$1
REMOTEHOST=linux024.student.cs.uwaterloo.ca
PRIVATEKEY=$2

if [ ! -z "$PRIVATEKEY" ]; then
	echo "Using the specified private key $PRIVATEKEY"
	PKOPT="-i $PRIVATEKEY"
fi

echo "***Do not edit***"
$HG commit -m "temp"

if [ $? -ne 0 ]; then
	echo "Just running remote build only"
	ssh $PKOPT $REMOTEUSER@$REMOTEHOST \
	"
		[ -f cs452.sh ] && source cs452.sh;
		cd cs452/kernel;
		hg update;
		./clean.sh;
		./build.sh;
	"	
	exit 0;
fi

TEMPCS=`$HG log | head -1 | awk '{ print $2 }' | cut -d ":" -f 2`
echo "The new temporary changeset: $TEMPCS"

$HG update -C $CURCS
$HG revert -a -r $TEMPCS
echo "***You may start editing your files again***"

$HG push -f -r $TEMPCS 
$HG strip -f $TEMPCS

echo "***Starting a remote build...***"
ssh $PKOPT $REMOTEUSER@$REMOTEHOST \
"
	[ -f cs452.sh ] && source cs452.sh;
	cd cs452/kernel;
	hg update -C $TEMPCS  > /dev/null;
	./clean.sh;
	./build.sh;
	hg update `hg parent | grep changeset: | awk '{print $2}' | cut -d ':' -f 1`
	hg strip $TEMPCS > /dev/null;
"


