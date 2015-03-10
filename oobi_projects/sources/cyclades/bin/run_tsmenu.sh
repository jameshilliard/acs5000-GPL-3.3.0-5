#!/bin/bash
#
#	File: /bin/run_tsmenu.sh
#
#	Description: Run ts_menu and exit
#
#	Usage: run_tsmenu.sh
#

if [ $# -ge 1 ]
then
	echo
	echo -e "\tUsage: run_tsmenu.sh"
	echo
	exit 1
fi

. /etc/profile

/bin/ts_menu

exit

#####################################################
