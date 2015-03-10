#!/bin/bash

KGEN=/bin/ssh-keygen
SAVE="no"

if [ ! -e /etc/ssh/ssh_host_key ] ; then
   	echo "Generating Hostkey..." 1> /dev/null 2>&1
   	${KGEN} -t rsa1 -b 1024 -f  /etc/ssh/ssh_host_key -N '' 1> /dev/null 2>&1
	SAVE="yes"
fi

if [ ! -e /etc/ssh/ssh_host_dsa_key ] ; then
   	echo "Generating DSA-Hostkey..." 1> /dev/null 2>&1
   	${KGEN} -d -b 1024 -f /etc/ssh/ssh_host_dsa_key -N '' 1> /dev/null 2>&1
	SAVE="yes"
fi

if [ ! -e /etc/ssh/ssh_host_rsa_key ] ; then
   	echo "Generating RSA-Hostkey..." 1> /dev/null 2>&1
   	${KGEN} -t rsa -b 1024 -f /etc/ssh/ssh_host_rsa_key -N '' 1> /dev/null 2>&1
	SAVE="yes"
fi

if [ "$SAVE" = "yes" ] ; then
	/bin/updatefiles /etc/ssh/sshd_config \
     	/etc/ssh/ssh_config \
     	/etc/ssh/ssh_host_key \
     	/etc/ssh/ssh_host_key.pub \
     	/etc/ssh/ssh_host_dsa_key \
     	/etc/ssh/ssh_host_dsa_key.pub \
     	/etc/ssh/ssh_host_rsa_key \
     	/etc/ssh/ssh_host_rsa_key.pub 1>/dev/null 2>&1
fi

if [ ! -e /etc/ssh/authorized_keys ] ; then
	touch /etc/ssh/authorized_keys
fi

