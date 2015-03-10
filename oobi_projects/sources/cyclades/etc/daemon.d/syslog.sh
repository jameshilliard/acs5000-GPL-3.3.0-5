# This file defines the syslog-ng configuration

ENABLE=YES        # Must be "NO" or "YES" (uppercase)
DNAME=syslog-ng   # daemon name
DPATH=/sbin       # daemon path
ShellInit=        # Performs any required initialization
ConfigFiles=/etc/syslog-ng/syslog-ng.conf      # configuration files
DTYPE=sig         # must be "sig" or "cmd" (lowercase)
DSIG=hup          # signal to stop/restart the daemon (lowercase)
                  # if it's hup term will be used to stop the daemon
# daemon command line parameters
DPARM=""
DSTOP=
