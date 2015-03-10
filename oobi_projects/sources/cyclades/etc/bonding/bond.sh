##################################################################################################
# bond.sh script - sthis scripts loads the bonding driver in case this feature was enabled
##################################################################################################
MIIMON=`cat /etc/bonding.opts | tr -d ' ' | grep MIIMONVALUE | cut -d= -f2`
UPDELAY=`cat /etc/bonding.opts | tr -d ' ' | grep UPDELAYVALUE | cut -d= -f2`
#mode = 1 --> active backup, what means that the pcmcia interface will act as a backup for fec
#-o bond0 --> the driver will be listed as bond0 when an lsmod is issued
#miimon --> interval in miliseconds in which the active interface is checked
#updelay -> time to wait after detecting an interface as up and before activating it. In our 
#case, it will represent the time in which the primary interface comes back after the link is
#restablished
case "$1" in
start)
	echo "Loading the bonding driver as bond0"
    K26=`set_nports 19`
    if [ "x$K26" = "x1" ] ; then
	  /sbin/modprobe bonding miimon=$MIIMON updelay=$UPDELAY \
         primary=eth0 mode=1 -o bond0
    else
      /sbin/insmod /lib/modules/2.4.17_mvl21-linuxplanet/kernel/drivers/net/bonding/bonding.o \
        miimon=$MIIMON updelay=$UPDELAY primary=eth0 mode=1 -o bond0
    fi
;;
stop)
	if lsmod | grep bond0 > /dev/null; then rmmod bond0; fi
;;
esac

