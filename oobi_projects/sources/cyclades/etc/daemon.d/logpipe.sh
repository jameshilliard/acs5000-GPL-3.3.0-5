# This file defines the pmd configuration

ENABLE=NO        # Must be "NO" or "YES" (uppercase)
DNAME=logpipe         # daemon name
DPATH=/usr/bin        # daemon path
# Performs any required initialization
ShellInit=
# configuration files
ConfigFiles="/etc/logpipe/logpipe.conf"
DTYPE=sig         # must be "sig" or "cmd" (lowercase)
DSIG=int          # signal to stop/restart the daemon (lowercase)
                  # if it's hup term will be used to stop the daemon
WAITK=YES         # Waits the process finish before starts again
# daemon command line parameters
DPARM=""
DSTOP=
