#!/bin/sh

#
# Checks if any the files related to a daemon was changed.
# A copy of the file is kept in the directory /var/run 
#
# Parameters: list of configuration files
#

CheckConf()
{
  rtn=1

  while [ -n "$1" ] ; do

    file=$(basename $1)

    cmp -s $1 /var/run/.$file.tmp > /dev/null 2>&1

    if [ $? -gt 0 ] ; then
      cp $1 /var/run/.$file.tmp
      rtn=0
    fi
    
    shift 1

  done

  return $rtn
}

#
# Starts/stops/restarts the daemon if configuration was changed. 
#
# Parameter: starting shell script for the daemon.
#
file=$1
  
#
# Clean up all variables
#
ENABLE=
DNAME=
DPATH=
ShellInit=
ConfigFiles=
DTYPE=
DSIG=
DPARM=
DSTOP=
WAITK=
PIDF=
noshowlog=0

. $file

# checks if start daemon shell still running
FILE_LOCK=/var/run/$3.lock
if [ -f $FILE_LOCK ] ; then
  kill -0 $(cat $FILE_LOCK)
  if [ $? -eq 0 ]; then
    echo "Error: $file still running!"
    exit 66
  fi
fi
echo $$ > $FILE_LOCK

case $2 in
init)
  noshowlog=1
  CheckConf $file $ConfigFiles

  if [ ! "x$ENABLE" = "xYES" ] ; then
    rm -f $FILE_LOCK
    exit
  fi
;;
check)
  CheckConf $file $ConfigFiles

  if [ $? -eq 1 ] ; then
    rm -f $FILE_LOCK
    exit
  fi
;;
restart)
  CheckConf $file $ConfigFiles
  ENABLE=YES
;;
stop) 
  WAITK=YES
  ENABLE=NO
  if [ "x$VERBOSE" = "xyes" ]; then
    echo "daemon.sh : stopping $DPATH/$DNAME"
  fi
;;
esac

#
# Daemon is handled by signal
#
if [ "x$DTYPE" = "xsig" ] ; then

  if [ -n "$PIDF" ] ; then
    if [ -f $PIDF ] ; then
       kill -0 $(cat $PIDF)
       if [ $? -eq 0 ] ; then
         MPID=$(cat $PIDF)
       fi
    fi
  else
    MPID=$(ps ax | grep "$DPATH/$DNAME" | grep -v "grep\|handle_daemon.sh" |\
            while read p1 p2 p3 p4 p5 p6 p7 p8 p9;\
            do \
              if [ "x$p5" = "x$DPATH/$DNAME" ] ; then \
                echo $p1;\
              fi \
            done)
  fi

  if [ -z "$DSIG" ] ; then
    DSIG=term
  fi

  if [ "$DSIG" = "hup" -a "x$ENABLE" = "xNO" ] ; then
    DSIG=term
  fi

  if [ -n "$MPID" ] ; then
    if [ "x$ENABLE" = "xYES" -a "$DSIG" = "hup" -a -n "$ShellInit" ] ; then
      $ShellInit
    fi
     kill -$DSIG $MPID
     if [ ! "$DSIG" = "hup" -a "x$WAITK" = "xYES" ]
     then
       cnt=0
       while true; do
         killall -q -0 $DNAME
         if [ $? -gt 0 ] ; then
           break
         fi
         sleep 1
         cnt=$(expr $cnt + 1)
         if [ $cnt -ge 30 ] ; then
           showlogmsg info "$0: daemon [$DNAME] could not be stopped"
           break
         fi
       done
     fi
     if [ $noshowlog -eq 0 ] ; then
	 sh_write_evt 12 Us $DNAME
     fi
  fi

  if [ "x$ENABLE" = "xYES" ] ; then

    if [ -z "$MPID" -o ! "$DSIG" = "hup" ] ; then
      if [ -n "$ShellInit" ] ; then
         $ShellInit
      fi

      if [ "x$VERBOSE" = "xno" ] ; then
        $DPATH/$DNAME $DPARM >/dev/null 2>&1 &
      else
        echo "daemon.sh : restarting $DPATH/$DNAME $DPARM"
        $DPATH/$DNAME $DPARM &
      fi
    fi
    if [ $noshowlog -eq 0 ] ; then
       sh_write_evt 11 Us $DNAME
    fi
  fi

  rm -f $FILE_LOCK
  exit

fi

#
# Daemon is handled by command (start/stop).
#
if [ "x$DTYPE" = "xcmd" ] ; then

  if [ "x$VERBOSE" = "xno" ] ; then
    $DPATH/$DNAME $DSTOP >/dev/null 2>&1
  else
    $DPATH/$DNAME $DSTOP 
  fi

  if [ $noshowlog -eq 0 ] ; then
     sh_write_evt 12 Us $DNAME
  fi

  if [ "x$ENABLE" = "xYES" ] ; then

    if [ -n "$ShellInit" ] ; then
      $ShellInit
    fi

    if [ "x$VERBOSE" = "xno" ] ; then
      $DPATH/$DNAME $DPARM >/dev/null 2>&1 
    else
      echo "restarting: $DPATH/$DNAME $DPARM"
      $DPATH/$DNAME $DPARM 
    fi
    if [ $noshowlog -eq 0 ] ; then
       sh_write_evt 11 Us $DNAME
    fi
  fi

  rm -f $FILE_LOCK
  exit

fi

