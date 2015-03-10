#!/bin/sh
stty opost onlcr ocrnl < /dev/console
if [ -f /etc/TIMEZONE ] ; then
TZ=`cat /etc/TIMEZONE`; export TZ
fi
echo > /dev/console
echo "*" > /dev/console
echo "* * test cron started" > /dev/console
echo "*" > /dev/console
echo > /dev/console
date > /dev/console
echo > /dev/console
echo "Sample of cron script that runs every minute" > /dev/console
echo > /dev/console
echo "*" > /dev/console
echo "* * test cron finished" > /dev/console
echo "*" > /dev/console
echo > /dev/console

