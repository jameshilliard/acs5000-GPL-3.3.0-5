#!/bin/sh

CONF_FILE="/etc/billing_up.conf"
BCRON_FILE="/etc/billing_crontab"
CRON_FILE="/etc/crontab_files"

. /etc/billing_up.conf

echo -n "Transfer Mode (ftp or scp)[$TRANSFER_MODE]: "
read resposta
TRANSFER_MODE=${resposta:-$TRANSFER_MODE}
echo -n "Local Directory[$LOCAL_DIRECTORY]: "
read resposta
LOCAL_DIRECTORY=${resposta:-$LOCAL_DIRECTORY}
echo -n "Remote server IP [$REMOTE_SERVER]: "
read resposta
REMOTE_SERVER=${resposta:-$REMOTE_SERVER}
echo -n "Remote directory [$REMOTE_DIRECTORY]: "
read resposta
REMOTE_DIRECTORY=${resposta:-$REMOTE_DIRECTORY}
echo -n "User [$USER]: "
read resposta
USER=${resposta:-$USER}
echo -n "Password [$PASSWORD]: "
read resposta
PASSWORD=${resposta:-$PASSWORD}
echo -n "Upload Interval in minutes [$UPLOAD_INTERVAL]: "
read resposta
UPLOAD_INTERVAL=${resposta:-$UPLOAD_INTERVAL}

	echo "#!/bin/sh" > $BCRON_FILE
	echo "" >> $BCRON_FILE
	echo "PATH=/bin:/usr/bin" >> $BCRON_FILE
	echo "SHELL=/bin/sh">> $BCRON_FILE
	echo "HOME=/root">> $BCRON_FILE
	
	echo "LOCAL_DIRECTORY=\"$LOCAL_DIRECTORY\"" > $CONF_FILE
	echo "REMOTE_DIRECTORY=\"$REMOTE_DIRECTORY\"">> $CONF_FILE
	echo "REMOTE_SERVER=\"$REMOTE_SERVER\"" >> $CONF_FILE
	echo "USER=\"$USER\"" >> $CONF_FILE
	echo "PASSWORD=\"$PASSWORD\"" >> $CONF_FILE
	echo "UPLOAD_INTERVAL=\"$UPLOAD_INTERVAL\"" >> $CONF_FILE
	echo "TRANSFER_MODE=\"$TRANSFER_MODE\"">> $CONF_FILE

	grep $BCRON_FILE $CRON_FILE >/dev/null 2>&1 || echo "active root $BCRON_FILE">> $CRON_FILE
	
if [ "$TRANSFER_MODE" = ftp ]; then

	echo "*/${UPLOAD_INTERVAL} * * * * /bin/billing_upload_ftp.sh >/dev/null 2>&1">> $BCRON_FILE

elif [ "$TRANSFER_MODE" = scp ]; then
	[ -f ~/.ssh/id_rsa.pub ]  || ssh-keygen -t rsa -f ~/.ssh/id_rsa -N ""
	ssh-copy-id -i ~/.ssh/id_rsa.pub "${USER}@${REMOTE_SERVER}"
	grep "/root/.ssh/id_rsa" /etc/config_files >/dev/null 2>&1|| echo "/root/.ssh/id_rsa" >> /etc/config_files
	grep "/root/.ssh/id_rsa.pub" /etc/config_files >/dev/null 2>&1|| echo "/root/.ssh/id_rsa.pub" >> /etc/config_files
	grep "/root/.ssh/known_hosts" /etc/config_files >/dev/null 2>&1|| echo "/root/.ssh/known_hosts" >> /etc/config_files
	echo "If your ssh server is a OpenSSH server, the key was uploaded to server."
	echo "Check if the server allow key authentication by this parameters: "
	echo " "
	echo "RSAAuthentication yes "
	echo "PubkeyAuthentication yes "
	echo "AuthorizedKeysFile  .ssh/authorized_keys "
	echo " "
	echo "If they are not like above, make the needed changes. If your server is"
	echo "not an OpenSSH server, check the provider documentation to configure and"
	echo "accept this and upload key. It is located in ~/.ssh/id_rsa.pub"
	echo "*/${UPLOAD_INTERVAL} * * * * /bin/billing_upload_ssh.sh > /dev/null 2>&1">> $BCRON_FILE

fi	
	echo "You must run saveconf and reboot the box to changes take effect."
cd $CWD
exit
