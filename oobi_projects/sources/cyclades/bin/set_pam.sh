#!/bin/sh
AWK=/bin/awk
GREP=/bin/grep
CUT=/usr/bin/cut
AUTH_FILE=/etc/portslave/pslave.conf
PAMD=/etc/pam.d
PAM_WEB=$PAMD/web
PAM_LOGIN=$PAMD/login
PAM_SSHD=$PAMD/sshd
PAM_PPP=$PAMD/ppp
PAM_IPPP=$PAMD/ippp
PAM_ADMIN=$PAMD/admin
RETC=0

$GREP "^conf.authtype" $AUTH_FILE > /dev/null 2>&1
if [ "$?" -ne "0" ]; then 
	AUTHTYPE=local
else
	AUTHTYPE=`$GREP "^conf.authtype" $AUTH_FILE | tail -n 1 | $AWK -F " " '{ print $2 }'`
	if [ "x$AUTHTYPE" = "xremote" ]; then
		AUTHTYPE=local
	fi
fi

PAM_MODULE=$PAMD/$AUTHTYPE

if [ -f $PAM_MODULE ]; then
	if [ -f $PAM_WEB ]; then
		rm -rf $PAM_WEB 
	fi
	if [ -f $PAM_LOGIN ]; then
		rm -rf $PAM_LOGIN
	fi
	if [ -f $PAM_SSHD ]; then
		rm -rf $PAM_SSHD 	
	fi
	if [ -f $PAM_IPPP ]; then
		rm -rf $PAM_IPPP
	fi
	ln -s $PAM_MODULE $PAM_WEB
	ln -s $PAM_MODULE $PAM_LOGIN
	ln -s $PAM_MODULE $PAM_SSHD
	ln -s $PAM_MODULE $PAM_IPPP

#assure admin user will always fallback to local authentication
# if service unavailable (<REMOTE>local)
	LOCAL=local
	echo "$AUTHTYPE" |grep local >/dev/null 2>&1
	if [ "$?" -eq "0" ]; then
		if [ -f $PAM_ADMIN ]; then
			rm -rf $PAM_ADMIN
		fi
		ln -s $PAM_MODULE $PAM_ADMIN
	else
		echo "$AUTHTYPE" |grep none >/dev/null 2>&1
		if [ "$?" -eq "0" ]; then
			if [ -f $PAM_ADMIN ]; then
				rm -rf $PAM_ADMIN
			fi
			ln -s $LOCAL $PAM_ADMIN
		else
			PAM_MODULE=$PAM_MODULE$LOCAL
			if [ -f $PAM_MODULE ]; then
				if [ -f $PAM_ADMIN ]; then
					rm -rf $PAM_ADMIN
				fi
				ln -s $PAM_MODULE $PAM_ADMIN
			else
				echo "can not find pam module for admin authentication!"
				RETC=$(($RETC+2))
			fi
		fi
	fi 
else
	echo "can not find pam module for unit authentication $AUTHTYPE !"
	RETC=$(($RETC+1))
fi

# PPP Authentication
$GREP "^conf.pppauthtype" $AUTH_FILE > /dev/null 2>&1
if [ "$?" -ne "0" ]; then 
	PPPAUTHTYPE=$AUTHTYPE
else
	PPPAUTHTYPE=`$GREP "^conf.pppauthtype" $AUTH_FILE | tail -n 1 | $AWK -F " " '{ print $2 }'`
	if [ "x$PPPAUTHTYPE" = "xunit-auth" ]; then
		PPPAUTHTYPE=$AUTHTYPE
	else
		if [ "x$PPPAUTHTYPE" = "xremote" ]; then
			PPPAUTHTYPE=local
		fi
	fi
fi

PAM_MODULE=$PAMD/$PPPAUTHTYPE

if [ -f $PAM_MODULE ]; then
	if [ -f $PAM_PPP ]; then
		rm -rf $PAM_PPP
	fi
	ln -s $PAM_MODULE $PAM_PPP
else
	echo "Can not find pam module for PPP authentication $PPPAUTHTYPE !"
	RETC=$(($RETC+1))
fi

