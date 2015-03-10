# This file defines the lpd configuration

ENABLE=NO        # Must be "NO" or "YES" (uppercase)
DNAME=lpd	  # daemon name
DPATH=/usr/sbin       # daemon path
ShellInit=        # Performs any required initialization
ConfigFiles="/etc/printcap /etc/hosts.lpd"   # configuration files
DTYPE=sig         # must be "sig" or "cmd" (lowercase)
DSIG=kill          # signal to stop/restart the daemon (lowercase)
                  # if it's hup term will be used to stop the daemon
# daemon command line parameters
DPARM=""
DSTOP=
