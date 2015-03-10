#!/bin/bash

DATE=`date '+%D %H:%M:%S'`
TIMESTAMP=`date '+%Y%m%d%H%M%S'`
TEMP=/tmp/.upgrade
LOGFILE=/tmp/upgrade_sh.log

#Check permission
if [ `id -u` -ne 0 ]
then
    echo "${DATE} - This shell script must be executed by root user." >> $LOGFILE
    exit 1
fi

#Check destination
if [[ ! -d /etc/daemon.d ]]
then
    echo "${DATE} - Directory /etc/daemon.d not found." >> $LOGFILE
    exit 1
fi

#Disable CRTL+C
trap "echo It cannot stop now. Please, wait." SIGINT

#Move files to destination
for files in cy_buffering.sh inetd.sh ipsec.sh kvmd.sh lpd.sh ntpclient.conf pmd.sh portmap.conf snmpd.sh sshd.sh syslog.shwebui.conf ypbind.conf
do
    if [ -f /etc/$files ]
    then
        echo "mv /etc/$files /etc/daemon.d/" >> $LOGFILE
        mv /etc/$files /etc/daemon.d/
    fi
done

#Update the references in /etc/config_files
if [ -f /etc/config_files ]
then
    egrep -v "cy_buffering.sh|inetd.sh|ipsec.sh|kvmd.sh|lpd.sh|ntpclient.conf|pmd.sh|portmap.conf|snmpd.sh|sshd.sh|syslog.sh|webui.conf|ypbind.conf" /etc/config_files > ${TEMP}
    echo "/etc/daemon.d/*" >> ${TEMP}
    cp /etc/config_files /etc/config_files.bkp_${TIMESTAMP}
    echo "${DATE} - Backup of config_files was done. It should be found in /etc/config_files.bkp_${TIMESTAMP}" >> $LOGFILE
    mv ${TEMP} /etc/config_files
else
    echo "${DATE} - The file /etc/config_files was not found." >> $LOGFILE
fi

#Update the references in /etc/daemon_list
if [ -f /etc/daemon_list ]
then
    cat /etc/daemon_list | sed -e "s/\/etc\//\/etc\/daemon.d\//g" > ${TEMP}
    cp /etc/daemon_list /etc/daemon_list.bkp_${TIMESTAMP}
    echo "${DATE} - Backup of daemon_list was done. It should be found in /etc/daemon_list.bkp_${TIMESTAMP}" >> $LOGFILE
    mv ${TEMP} /etc/daemon_list
else
    echo "${DATE} - The file /etc/daemon_list was not found." >> $LOGFILE
fi

# Activate shadow
if [ -z "`grep -s 'root:x:0:0:' /etc/passwd`" ] ; then
  rm -f /etc/shadow
  pwconv
fi


#Validate all modification
if [ -x /bin/saveconf ]
then
    /bin/saveconf >> $LOGFILE

    if [ -x /bin/daemon.sh ]
    then
        /bin/daemon.sh restart >> $LOGFILE
    else
        echo "${DATE} - The file daemon.sh command not found." >> $LOGFILE
    fi
else
    echo "${DATE} - The file saveconf command not found." >> $LOGFILE
fi

trap "" SIGINT
echo "Done!!! Please, check log file located in ${LOGFILE}."
