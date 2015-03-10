#!/bin/sh

#
# Checks if any the files related to a daemon was changed.
# A copy of the file is kept in the directory /var/run 
#
# Parameters: list of configuration files
#

export VERBOSE=no

#
# Reads the daemon_list file and performs the proper action ($1).
#
# If $2 was given it is the service id.
#
process_file()
{
  COMMAND=$2
  grep -v ^# /etc/daemon_list | while read nickname dfile 
  do
    if [ -z "$nickname" ] ; then
      continue
    fi
    if [ -z "$COMMAND" -o "x$COMMAND" = "x$nickname" ] ; then
      if [ "$1" = "stop" ] ; then
        /bin/handle_daemon.sh $dfile $1 $nickname
      else
        /bin/handle_daemon.sh $dfile $1 $nickname &
      fi
      if [ -n "$COMMAND" ] ; then
        return 55
      fi
    fi
  done
#[RK]Dec/22/03 - bug#1215 daemon.sh: No error checking, no feedback at all
  if [ "x$COMMAND" != "x" -a "x$?" != "x55" ]; then
	echo "The service_id $2 is invalid."
	print_help
	exit
  fi
}

#
# Displays the help message
#
print_help()
{
  echo
  echo "USAGE : daemon.sh [-h|help] | [{[stop|restart] service_id}...]"
  echo
  echo "where service_id can be any choice of:"
  echo
  grep -v ^# /etc/daemon_list | while read nickname dfile 
  do
    echo -n "$nickname "
  done
  echo
}

#
# If no parameters were given just checks for configuration changes 
#
if [ $# -eq 0 ] ; then
  process_file check
  exit
fi
 
#
# Called from init
#
if [ "$1" = "init" ] ; then
  process_file init
  exit
fi

#
# Just display help message
#
if [ "x$1" = "x-h" -o "x$1" = "xhelp" ] ; then
  print_help
  exit
fi

if [ "x$1" = "x-v" ] ; then
  VERBOSE=yes
  if [ $# -eq 1 ] ; then
    process_file check
    exit
  fi
fi

#
# Default action
#
action=restart

#
# Starts/stops the solicited services
#
for parm in $*
do
  if [ "x$parm" != "x-v" ] ; then
     case $parm in
     restart|stop)
        action=$parm
        ;;
     *)
        process_file $action $parm
        ;;
     esac
  fi
done

