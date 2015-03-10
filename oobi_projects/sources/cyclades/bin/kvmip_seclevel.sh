#!/bin/sh
SEC=$1
BRDS=$2
STEP=1

if [ $BRDS -gt 4 ]; then
	exit 1
fi

while [ $STEP -le $BRDS ]; do
	/usr/bin/proxy post-security-setting $SEC $STEP
	STEP=$(($STEP+1))
done

exit 0
