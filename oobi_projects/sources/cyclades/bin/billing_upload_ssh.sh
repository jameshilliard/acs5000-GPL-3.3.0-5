#!/bin/sh

. /etc/billing_up.conf

CWD=`pwd`

LOG_FILE="/tmp/billing_ssh_log"

cd $LOCAL_DIRECTORY

if echo "$REMOTE_SERVER" | egrep -q '^[0-9a-fA-F]*:[0-9a-fA-F:]*([0-9.]*)?(%.*)?$'; then
	# IPv6 address must be inside brackets...
	REMOTE_SERVER="[$REMOTE_SERVER]"
fi

ls *.txt >> /dev/null 2>&1 || exit 0

ls *.txt | while read line; do 
	if scp $line "$USER@$REMOTE_SERVER:$REMOTE_DIRECTORY"; then
		rm $line
	fi
done
