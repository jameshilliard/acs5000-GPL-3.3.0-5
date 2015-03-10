#!/bin/sh

BRD="$1"

if [ "x$BRD" = "x" ]; then
	BRD=0
fi

# Download the Viewer from the board in the script argument or 
# from the first KVM/IP Module. No need to try other
# modules once the file would be overwritten
if [ "$BRD" = "0" ]; then
	if /usr/bin/proxy download-vpclient 1 > /dev/null 2>&1; then
		echo "Downloaded KVM/IP Module #1 Viewer"
	elif /usr/bin/proxy download-vpclient 2 > /dev/null 2>&1; then
		echo "Downloaded KVM/IP Module #2 Viewer"
	elif /usr/bin/proxy download-vpclient 3 > /dev/null 2>&1; then
		echo "Downloaded KVM/IP Module #3 Viewer"
	elif /usr/bin/proxy download-vpclient 4 > /dev/null 2>&1; then
		echo "Downloaded KVM/IP Module #4 Viewer"
	fi
else
	/usr/bin/proxy download-vpclient $BRD > /dev/null 2>&1
fi

