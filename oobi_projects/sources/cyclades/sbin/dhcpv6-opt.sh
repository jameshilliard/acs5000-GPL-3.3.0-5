#!/bin/sh

DNSCONF=/etc/resolv.conf
DNSCONF_TMP=/tmp/resolv.conf.tmp
OPTSET=/var/run/dhcpv6-opt.set

source /etc/dhcpv6-opt.conf

if [ "x$DNS" == "xyes" ]; then
	if [ -n "$new_domain_name_servers" ]; then
		grep $new_domain_name_servers $DNSCONF > /dev/null 2>&1
		if [ $? -ne 0 ]; then 
			echo "nameserver $new_domain_name_servers" >> $DNSCONF
			echo "$DNSCONF:nameserver $new_domain_name_servers" >> $OPTSET
		fi
	fi
fi
if [ "x$DOMAIN" == "xyes" ]; then
	if [ -n "$new_domain_name" ]; then
		grep $new_domain_name $DNSCONF > /dev/null 2>&1
		if [ $? -ne 0 ]; then
			echo "domain $new_domain_name" >> $DNSCONF
			echo "$DNSCONF:domain $new_domain_name" >> $OPTSET
		fi 
	fi
fi

#remove trailing spaces
sed 's/[ \t]*$//' $DNSCONF >> $DNSCONF_TMP
cp $DNSCONF_TMP $DNSCONF
rm -f $DNSCONF_TMP 
