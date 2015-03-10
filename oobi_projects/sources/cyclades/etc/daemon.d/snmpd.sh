# This file defines the snmpd service configuration

ENABLE=YES        # Must be "NO" or "YES" (uppercase)
DNAME=snmpd       # daemon name
DPATH=/sbin       # daemon path
ShellInit="rm -f /var/net-snmp/snmpd.conf"
# configuration files
ConfigFiles="/etc/snmp/snmpd.conf /etc/snmp/snmpd.local.conf"
DTYPE=sig         # must be "sig" or "cmd" (lowercase)
DSIG=kill         # signal to stop/restart the daemon (lowercase)
                  # if it's hup term will be used to stop the daemon
# daemon command line parameters
DPARM=""
DSTOP=
