#!/bin/sh

PFILE="/etc/portslave/pslave.conf"
PFILE_TEMP="/tmp/pslave.tmp"
BCONF="/etc/billing_up.conf"

usage () {
	echo "Usage: config_billing X [options]"
	echo "where: "
	echo "X is the port's number which is configured"
	echo "and [options] can be:"
	echo "	-s     speed"
	echo "	-d     data size"
	echo "	-b     stopbit"
	echo "	-p     parity"
	echo "	-r     billing records"
	echo "	-e     billing EOR(this option must be on \" \", like \"\\n\")"
	echo "	-D     billing dir"
	echo "	-S     serverFarm"
	echo "	-t     date"
	echo "	-T     timeout"
	echo "	-i     ip"
	echo "	-n     netmask"
	echo "	-R     route"
	echo "	-u     upload"

	rm $PFILE_TEMP >> /dev/null 2>&1
	exit 0
}

[ $# -lt 1 ] && usage

 echo $1 | grep -vq "-" && TTY=$1 && shift

#Enable port
[ -z "$TTY" ] && echo "You need specify the port to use this option" && usage
sed -e "s/\#s${TTY}\.tty/s${TTY}\.tty/g" ${PFILE} > ${PFILE_TEMP}
mv $PFILE_TEMP $PFILE

while [ $# -ne 0 ]; do
	case $1 in
		-d) 
			[ -z "$TTY" ] && echo "You need specify the port to use this option" && usage
			shift
			[ ! "$1" ] && usage
			if grep s${TTY}.datasize $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.datasize/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.datasize\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.datasize\t${1}" >> $PFILE
			fi
			;;
		-s) 
			[ -z "$TTY" ] && echo "You need specify the port to use this option" && usage
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
		-b)
			[ -z "$TTY" ] && echo "You need specify the port to use this option" && usage
			 shift
			[ ! "$1" ] && usage
			if grep s${TTY}.stopbits $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.stopbits/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.stopbits\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.stopbits\t${1}" >> $PFILE
			fi
			;;
		-p)
			[ -z "$TTY" ] && echo "You need specify the port to use this option" && usage
			shift
			[ ! "$1" ] && usage
			if grep s${TTY}.parity $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.parity/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.parity\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.parity\t${1}" >> $PFILE
			fi
			;;
		-r)
			[ -z "$TTY" ] && echo "You need specify the port to use this option" && usage
			shift
			[ ! "$1" ] && usage
			if grep s${TTY}.billing_records $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.billing_records/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.billing_records\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.billing_records\t${1}" >> $PFILE
			fi
			;;
		-e)
			[ -z "$TTY" ] && echo "You need specify the port to use this option" && usage
			shift
			[ ! "$1" ] && usage
			if grep s${TTY}.billing_eor $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.billing_eor/d" $PFILE > $PFILE_TEMP
				echo -en "s${TTY}.billing_eor\t" >> $PFILE_TEMP
				echo \"$1\" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -en "s${TTY}.billing_eor\t" >> $PFILE
				echo \"$1\" >> $PFILE
			fi
			;;
		-D)
			[ -z "$TTY" ] && echo "You need specify the port to use this option" && usage
			shift
			[ ! "$1" ] && usage
			if grep s${TTY}.billing_dir $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.billing_dir/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.billing_dir\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.billing_dir\t${1}" >> $PFILE
			fi
			;;
		-S)
			[ -z "$TTY" ] && echo "You need specify the port to use this option" && usage
			shift
			[ ! "$1" ] && usage
			if grep s${TTY}.serverfarm $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.serverfarm/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.serverfarm\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.serverfarm\t${1}" >> $PFILE
			fi
			;;
		-t) 
			shift
			[ ! "$1" ] && usage
			date $1
			;;
		-T)
			[ -z "$TTY" ] && echo "You need specify the port to use this option" && usage
			shift
			[ ! "$1" ] && usage
			if grep s${TTY}.billing_timeout $PFILE >> /dev/null 2>&1; then
				sed "/s${TTY}.billing_timeout/d" $PFILE > $PFILE_TEMP
				echo -e "s${TTY}.billing_timeout\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "s${TTY}.billing_timeout\t${1}" >> $PFILE
			fi
			;;
		-i)
			shift
			[ ! "$1" ] && usage
			if grep conf.eth_ip $PFILE >> /dev/null 2>&1; then
				sed "/conf.eth_ip/d ; /conf.dhcp/d" $PFILE > $PFILE_TEMP
				echo -e "conf.eth_ip\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "conf.eth_ip\t${1}" >> $PFILE
			fi
			IP=1
			;;
		-n)
			shift
			[ ! "$1" ] && usage
			if grep conf.eth_mask $PFILE >> /dev/null 2>&1; then
				sed "/conf.eth_mask/d" $PFILE > $PFILE_TEMP
				echo -e "conf.eth_mask\t${1}" >> $PFILE_TEMP
				mv $PFILE_TEMP $PFILE
			else
				echo -e "conf.eth_mask\t${1}" >> $PFILE
			fi
			IP=1
			;;
		-R)
			shift
			[ ! "$1" ] && usage
			sed "/default/d" /etc/network/st_routes > /tmp/route
			echo "route add default gw $1 dev eth0 metric 3" >> /tmp/route
			mv /tmp/route /etc/network/st_routes
			IP=1
			;;
		-u)
			if [  "$TTY" ]   || { cat $PFILE | grep protocol | grep billing >> /dev/null 2>&1; } ; then
				billing_upload_files.sh 
			else
				echo "There is no port configured as billing"
			fi
			;;
		*)
			usage
			;;
	esac
	shift
done

if [ "$TTY" ]; then
	sed "/s${TTY}.authtype/d ; /s${TTY}.protocol/d ; /s${TTY}.flow/d ; /s${TTY}.dcd/d ; /s${TTY}.sniff_mode/d" $PFILE > $PFILE_TEMP
	echo "s${TTY}.authtype none">> $PFILE_TEMP
	echo "s${TTY}.protocol billing">> $PFILE_TEMP
	echo "s${TTY}.flow none">> $PFILE_TEMP
	echo "s${TTY}.dcd 0">> $PFILE_TEMP
	echo "s${TTY}.sniff_mode no">> $PFILE_TEMP
	PORT=`cat /proc/bdinfo | cut -f 2 -d \!`
	if [ "$PORT" = "1" ]; then
		sed "s/^ttyS0/#ttyS0/" /etc/inittab > /etc/inittab.new
		mv /etc/inittab.new /etc/inittab
	fi
	mv $PFILE_TEMP $PFILE
	rm $PFILE_TEMP >> /dev/null 2>&1
fi
[ "$TTY" -o "$IP" ] && signal_ras hup
