#!/bin/sh
#
# Checks if PMD is alive and restarts it if needed.
#

# get variables
. /etc/daemon.d/pmd.sh

# check if there is another PMD Monitor running
if [ `ps | grep pmdmonitor.sh | grep -v grep | wc -l` -gt 2 ]; then
	exit
fi

# main loop
while true; do
sleep 5
if [ -f $PIDF ]; then
	kill -0 $(cat $PIDF) > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		daemon.sh restart PMD
		#log action
		if [ `cat /var/log/pmdmonitor.log | wc -l` -gt 1000 ]; then
			#prevents running out of ramdisk
			rm -f /var/log/pmdmonitor.log
		fi
		date >> /var/log/pmdmonitor.log
	fi
else
	exit
fi
done
