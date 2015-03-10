#!/bin/sh

PSLAVE_TMP=/etc/portslave/pslave.tmp
PSLAVE=/etc/portslave/pslave.conf
PMD_CONF=/etc/pmd.conf
PMD_USER=/etc/pmd.users
PMD_GRP=/etc/pmdgrp.conf
AWK_SCR=/bin/awk_power

PRODUCT=`cut -d '!' -f 3 /proc/bdinfo`
if [ "$PRODUCT" = "ACS" ]; then
        mv ${PSLAVE} ${PSLAVE_TMP}
else
        sed "s/^\([#]\?\)s\([12]\.\)/\1a\2/g" ${PSLAVE} > ${PSLAVE_TMP}
fi
rm -f ${PSLAVE}

awk -f ${AWK_SCR} \
PSLAVE=${PSLAVE} \
PMD_CONF=${PMD_CONF} \
PMD_USER=${PMD_USER} \
PMD_GRP=${PMD_GRP} \
${PSLAVE_TMP}

# upgrade the /etc/daemon.d/pmd.sh file
ENABLE=
. /etc/daemon.d/pmd.sh

NEWVAL="ENABLE="$ENABLE
OLDVAL="ENABLE=YES"
sed "s/$OLDVAL/$NEWVAL/" /etc/daemon.d/pmd.sh.save > /etc/daemon.d/pmd.sh
