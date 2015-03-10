******** DLOG ************

This package was copied from SVN:

https://fre-svn.corp.avocent.com/svn/avocent-gsp/sources/generic/dlog/trunk


Debug logging API for GSP Appliances.

The API may be built as a shared library, or static library.

In addition, a command-line program for editing the log rules may be built.

Finally, the dlog.h file should be included in both user and kernel space
programs.

The DLOG will by default be configured to output to Syslog, and log all
WARNINGs and above for all applications. 

"dlogcfg refresh" should be executed after "insmod" to make sure kernel modules
obtain their correct logging level.

** Environmental Dependencies: 

This implementation makes the following assumptions about the OS Environment:
	sysfs is mounted in /sys - This is used to configure kernel modules
	Syslog is listening for Unix Datagrams on /dev/log - This is used
for Syslog logging.
