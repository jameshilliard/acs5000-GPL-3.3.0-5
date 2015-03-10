# This file defines whether services in inetd.conf are enabled or disabled

ENABLE=YES        # Must be "NO" or "YES" (uppercase)
DNAME=inetd       # daemon name
DPATH=/bin        # daemon path
ShellInit=        # Performs any required initialization
# configuration files
ConfigFiles=/etc/inetd.conf
DTYPE=sig         # must be "sig" or "cmd" (lowercase)
DSIG=term          # signal to stop/restart the daemon (lowercase)
                  # if it's hup term will be used to stop the daemon
# daemon command line parameters
DPARM=""
DSTOP=
