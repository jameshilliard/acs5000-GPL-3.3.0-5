#!/bin/sh

. /etc/ntpclient.conf

if [ "$NTPSERVER" != "" ]; then
	/bin/ntpclient -s -c 0 -i $INTERVAL -h $NTPSERVER >/dev/null 2>&1 &
fi

