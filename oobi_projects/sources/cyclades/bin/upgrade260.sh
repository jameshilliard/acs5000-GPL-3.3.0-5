#!/bin/bash
# Upgrade script for device's software V2.6.0

DATE=`date '+%D %H:%M:%S'`
TIMESTAMP=`date '+%Y%m%d%H%M%S'`
TEMP=/tmp/.upgrade
NEEDSAVE=0

if [ -f /etc/cy_buffering.sh ]
then 
    #Move files to destination
    for files in cy_buffering.sh inetd.sh ipsec.sh kvmd.sh lpd.sh ntpclient.conf pmd.sh portmap.conf snmpd.sh sshd.sh syslog.sh webui.conf ypbind.conf
    do
        if [ -f /etc/$files ]
        then
            mv /etc/$files /etc/daemon.d/ 
        fi
    done

mv /etc/daemon.d/webui.conf /etc/daemon.d/webui.tmp
awk  '$1 ~ /^ShellInit=/ { print "ShellInit=/bin/firstkssl.sh   # Performs any required initialization"; next}; { print $0 };' /etc/daemon.d/webui.tmp > /etc/daemon.d/webui.conf

    #Update the references in /etc/config_files
    if [ -f /etc/config_files ]
    then
         egrep -v "cy_buffering.sh|inetd.sh|ipsec.sh|kvmd.sh|lpd.sh|ntpclient.conf|pmd.sh|portmap.conf|snmpd.sh|sshd.sh|syslog.sh|webui.conf|ypbind.conf" /etc/config_files > ${TEMP}
         echo "/etc/daemon.d/*" >> ${TEMP}
         cp /etc/config_files /etc/config_files.bkp_${TIMESTAMP}
         echo "${DATE} - Backup of config_files was done. It should be found in /etc/config_files.bkp_${TIMESTAMP}" 
         mv ${TEMP} /etc/config_files
    else
         echo "${DATE} - The file /etc/config_files was not found." >> $LOGFILE
    fi
    NEEDSAVE=1
fi

# Add new shared memory init to inittab
if [ -f /etc/inittab ]; then
        if ! grep -q updconf /etc/inittab; then
                echo "# initialize shared memory configuration" >> /etc/inittab
                echo "::wait:/bin/updconf -f -q" >> /etc/inittab
                echo "Updated /etc/inittab" 
		NEEDSAVE=1
        fi
fi

# Correct TIMEZONE var in /etc/daemon.d/ntpclient.conf, if needed
if [ -f /etc/daemon.d/ntpclient.conf ]; then
    TZadj=`grep TIMEZONE /etc/daemon.d/ntpclient.conf`
    if [ $TZadj ]; then
        TZadj=${TZadj:9}
        if [ "x$TZadj" != "xCustom" ] && [ "x${TZadj:0:3}" != "xgmt" ]; then
            if [ "x${TZadj:3:1}" = "x+" ]; then
                TZadj=gmt-${TZadj:4}
            elif [ "x${TZadj:3:1}" = "x-" ]; then
                TZadj=gmt+${TZadj:4}
            else
                TZadj=gmt
            fi
            grep -v TIMEZONE /etc/daemon.d/ntpclient.conf > /tmp/ntpclient.conf 2>&1
            echo "TIMEZONE=$TZadj" >> /tmp/ntpclient.conf
            mv /tmp/ntpclient.conf /etc/daemon.d
            echo "TIMEZONE var was adjusted to the new format in /etc/daemon.d/ntpclient.conf"
            NEEDSAVE=1
        fi
    fi
fi

#Validate all modification
if [ $NEEDSAVE = 1 ]
then
    /bin/saveconf 
fi
