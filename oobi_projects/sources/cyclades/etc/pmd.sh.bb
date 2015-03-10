# This file defines the pmd configuration

ENABLE=YES        # Must be "NO" or "YES" (uppercase)
DNAME=pmd         # daemon name
DPATH=/bin        # daemon path
# Performs any required initialization
ShellInit=""
# configuration files
ConfigFiles="/etc/pm.bb"
DTYPE=sig         # must be "sig" or "cmd" (lowercase)
DSIG=int          # signal to stop/restart the daemon (lowercase)
                  # if it's hup term will be used to stop the daemon
WAITK=YES         # Waits the process to finish before starts a new one
# daemon command line parameters
DPARM=""
DSTOP=
