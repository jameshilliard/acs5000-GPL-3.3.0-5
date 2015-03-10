# This file defines the ipsec service configuration

ENABLE=NO                   # Must be "NO" or "YES" (uppercase)
DNAME=ipsec                 # daemon name
DPATH=/usr/local/sbin       # daemon path
ShellInit=/etc/ipsec.init   # Performs any required initialization
ConfigFiles=                # configuration files
DTYPE=cmd                   # must be "sig" or "cmd"
DSIG=kill         # signal to stop/restart the daemon (lowercase)
                  # if it's hup term will be used to stop the daemon
# daemon command line parameters
DPARM="setup --start"
DSTOP="setup --stop"

