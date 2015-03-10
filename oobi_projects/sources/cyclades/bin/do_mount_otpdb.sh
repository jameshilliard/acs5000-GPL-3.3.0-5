#!/bin/sh

OTP_CONF=/etc/otp.conf
IDE_CONF=/etc/pcmcia/ide.opts

do_mount ()
{
  eval mount $1 $2 $3
  if [ $? -ne 0 ] ; then
    echo "*"
    echo "* ERROR - could not mount '$2' in '$3' ..."
    echo "*"
    exit 1
  fi
}

if [ ! -f ${OTP_CONF} ] ; then
  echo "*"
  echo "* ERROR - Configuration file '${OTP_CONF}' does not exist."
  echo "*"
  exit 1
fi

if [ "x$1" = "xinit" ] ; then
  shift
  while true
  do
    MPID=$(ps ax | grep "/sbin/portmap" | grep -v grep | cut -b -6)
    if [ -z "$MPID" ] ; then
      sleep 1
      continue
    fi
    kill -0 $MPID
    if [ $? -eq 0 ] ; then
      break;
    fi
    sleep 1
  done
fi

. ${OTP_CONF}

umount ${MOUNT_POINT} >/dev/null 2>&1

if [ "x$1" = "xumount" ] ; then
  exit 0
fi

if [ ! "x${ENABLE}" = "xYES"  -a \
     ! "x$1" = "xforce" ] ; then
	exit 1
fi

rm -rf ${MOUNT_POINT}
case ${DEVICE} in
LOCAL)
  if [ -n "`grep -s '=/dev/ram' /proc/cmdline`" ] ; then
    MY_DIR=/
  else
    MY_DIR=/mnt/hdCnf
  fi
  [ -d ${MOUNT_POINT} ] || mkdir -p ${MOUNT_POINT}
  do_mount --bind ${MY_DIR} ${MOUNT_POINT}
  ;;
PCMCIA)
  MOUNTPT=$(awk '/MOUNTPT=/ { print $2 }' FS='"' ${IDE_CONF})
  FSTYPE=$(awk '/FSTYPE=/ { print $2 }' FS='"' ${IDE_CONF})

  if [ -z "${MOUNTPT}" -o -z "${FSTYPE}" ]; then
    echo "*"
    echo "* ERROR - Missing definition of 'MOUNTPT' or 'FSTYPE' in 'ide.opts'"
    echo "*"
    exit 1
  fi

  if [ "x$(set_nports 18)" = "x1" ] ; then
    MY_DEVICE=/dev/hdc1
  else
    MY_DEVICE=/dev/hda1
  fi

  [ -d ${MOUNTPT} ] || mkdir -p ${MOUNTPT}
  ln -sf ${MOUNTPT} ${MOUNT_POINT}
  if [ -z "$(mount | grep -s ${MY_DEVICE})" ] ; then
    do_mount "-t ${FSTYPE}" ${MY_DEVICE} ${MOUNTPT}
  fi
  ;;
*)
  [ -d ${MOUNT_POINT} ] || mkdir -p ${MOUNT_POINT}
  do_mount "-o intr,soft -t nfs" ${DEVICE} ${MOUNT_POINT}
  ;;
esac

