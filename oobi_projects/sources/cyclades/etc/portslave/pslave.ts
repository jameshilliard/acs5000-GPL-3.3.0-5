#
# pslave.conf	Sample server configuration file.
#
#  Terminal Server Profile


conf.eth_ip	    200.200.200.1
conf.eth_mask	255.255.255.0
conf.eth_mtu	1500
conf.lockdir	/var/lock
conf.rlogin	/usr/local/bin/rlogin-radius
conf.telnet	/usr/bin/telnet
conf.ssh	/bin/ssh
conf.locallogins	0

all.speed       9600
all.datasize    8
all.stopbits     1
all.parity      none
all.authtype    none
all.protocol	telnet
all.socket_port	23
all.telnet_client_mode		0

all.host	200.200.200.3
all.issue       \r\n\
     Welcome to terminal server %h port S%p \n\
\r\n

all.prompt	%h login: 
all.term	vt100
all.flow	hard

#
# Users joe and mark will only have access granted to the serial port ttyS5
# 
all.users ! joe, mark

#
# Port-specific parameters
#
s1.tty			ttyS1

s2.tty			ttyS2
s2.authtype		local
s2.protocol		rlogin
s2.speed		19200
s2.datasize		7
s2.stopbits		2
s2.parity		even

s3.tty			ttyS3
s3.protocol		ssh2
s3.authtype		remote

s4.tty			ttyS4
s4.protocol		ssh
s4.authtype		remote

s5.tty			ttyS5
s5.users   joe, mark

