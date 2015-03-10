#!/bin/sh

PFILE="/etc/portslave/pslave.conf"
PFILE_TEMP="/tmp/pslave.tmp"

usage () {
	echo " Usage : wizRawData [-i <IP address> <netmask>] [ <serial port> [-b <baudrate>] [-d <data length>] [-s <stop bits>] [-p <parity>] [-f  [flow control] [-P <protocol >] [-II <IP address>] [-IP <the TCP port>] [-OI <IP address>] [-OP <TCP port>] [-OT <idle time>]]"
	echo " options :"
	echo "  -i <IP address> <netmask> : IP address and mask of TS"
	echo "  <serial port>     : the number of serial port that will be configured" 
	echo "  The following options are associated with the serial port defined by <serial port>"
	echo "   -b <baudrate>    : baud rate"
	echo "   -d <data length> : data length - 4,5,6,7 or 8"
	echo "   -s <stop bits>   : stop bits - 1 or 2"
	echo "   -p <parity>      : parity - none, odd or even"
	echo "   -f <flow control>: flow control - none, hard or soft"
	echo "   -P <protocol>    : protocol - raw_data or raw_data_2way"
	echo "   -II <IP address> : define the IP address for incoming connection"
	echo "   -IP <TCP port>   : define the TCP port for the incoming connection"
	echo "   -OI <IP address> : define the IP address for outgoing connection"
	echo "   -OP <TCP port>   : define the TCP port for outgoind connection"
	echo "   -OT <idle time>  : define the idle time for outgoing connection"

	rm $PFILE_TEMP >> /dev/null 2>&1
	exit 0
}

[ $# -lt 1 ] && usage
[ $1 = -h ] && usage

if [ $1 = -i ]; then
	shift
	[ ! "$1" ] && usage
	if grep conf.eth_ip $PFILE >> /dev/null 2>&1; then
		sed "/conf.eth_ip/d ; /conf.eth_mask/d ; /conf.dhcp/d " $PFILE > $PFILE_TEMP
		echo -e "conf.eth_ip\t${1}" >> $PFILE_TEMP
		echo -e "conf.eth_mask\t${2}" >> $PFILE_TEMP
		mv $PFILE_TEMP $PFILE
	else
		echo -e "conf.eth_ip\t${1}" >> $PFILE
		echo -e "conf.eth_mask\t${2}" >> $PFILE
	fi
	shift
	shift
	if [ $# -eq 0 ]; then
		exit 0
	fi
fi

echo $1 | grep -vq "-" && TTY=$1 && shift

[ -z "$TTY" ] && echo "You must specify the number of the serial port" && usage

while [ $# -ne 0 ]; do
	case $1 in
		-b) 
			shift
			[ ! "$1" ] &&  usage
			if grep s${TTY}.speed $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.speed/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.speed\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.speed\t${1}" >> $PFILE
			fi
			;;
		-d) 
			shift
			[ ! "$1" ] && usage
			if [ $1 != 8 -a $1 != 7 -a $1 != 6 -a $1 != 5 -a $1 != 4 ];then
				usage
			fi
			if grep s${TTY}.datasize $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.datasize/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.datasize\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.datasize\t${1}" >> $PFILE
			fi
			;;
		-s)
			 shift
			[ ! "$1" ] && usage
			if [ $1 != 1 -a $1 != 2 ]; then
				usage
			fi
			if grep s${TTY}.stopbits $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.stopbits/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.stopbits\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.stopbits\t${1}" >> $PFILE
			fi
			;;
		-p)
			shift
			[ ! "$1" ] && usage
			if [ $1 != none -a $1 != odd -a $1 != even ]; then
				usage
			fi
			if grep s${TTY}.parity $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.parity/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.parity\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.parity\t${1}" >> $PFILE
			fi
			;;
		-f)
			shift
			[ ! "$1" ] && usage
			if [ $1 != none -a $1 != hard -a $1 != soft ]; then
				usage
			fi
			if grep s${TTY}.flow $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.flow/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.flow\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.flow\t${1}" >> $PFILE
			fi
			;;
		-P)
			shift
			[ ! "$1" ] && usage
			if [ $1 != raw_data -a $1 != raw_data_2way ]; then
				usage
			fi
			if grep s${TTY}.protocol $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.protocol/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.protocol\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.protocol\t${1}" >> $PFILE
			fi
			;;
		-IP)
			shift
			[ ! "$1" ] && usage
			if grep s${TTY}.socket_port $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.socket_port/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.socket_port\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.socket_port\t${1}" >> $PFILE
			fi
			;;
		-II)
			shift
			if grep s${TTY}.ipno $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.ipno/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.ipno\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.ipno\t${1}" >> $PFILE
			fi
			;;
		-OI)
			shift
			[ ! "$1" ] && usage
			if grep s${TTY}.host $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.host/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.host\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.host\t${1}" >> $PFILE
			fi
			;;
		-OP)
			shift
			[ ! "$1" ] && usage
			if grep s${TTY}.host_port $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.host_port/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.host_port\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.host_port\t${1}" >> $PFILE
			fi
			;;
		-OT)
			shift
			[ ! "$1" ] && usage
			if grep s${TTY}.host_idletime $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.host_idletime/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.host_idletime\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.host_idletime\t${1}" >> $PFILE
			fi
			;;
		*)
			usage
			;;
	esac
	shift
done

if [ "$TTY" ]; then
	sed "/s${TTY}.authtype/d ; /s${TTY}.dcd/d ; /s${TTY}.sniff_mode/d" $PFILE > $PFILE_TEMP
	echo "s${TTY}.authtype none">> $PFILE_TEMP
	echo "s${TTY}.dcd 0">> $PFILE_TEMP
	echo "s${TTY}.sniff_mode no">> $PFILE_TEMP
	mv $PFILE_TEMP $PFILE
	rm $PFILE_TEMP >> /dev/null 2>&1
fi

echo "To run this configuration : \"signal_ras hup\""
echo "To save the configuration in flash : \"saveconf\""
