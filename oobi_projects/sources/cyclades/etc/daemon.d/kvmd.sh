# This file defines the kvmd configuration

ENABLE=YES     # Must be "NO" or "YES" (uppercase)
DNAME=kvmd     # daemon name
DPATH=/bin     # daemon path
ShellInit=""   # Performs any required initialization
ConfigFiles="/etc/kvmd.conf /etc/kvmd.slaves /etc/daemon.d/ypbind.conf"   # configuration files
DTYPE=sig      # must be "sig" or "cmd"
DSIG=term       # signal to stop/restart the daemon (lowercase)
               # if it's hup term will be used to stop the daemon
# daemon command line parameters
DPARM=""
DSTOP=
