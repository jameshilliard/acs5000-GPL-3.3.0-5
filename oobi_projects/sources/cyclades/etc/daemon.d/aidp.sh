# This file defines the aidpd configuration

ENABLE=YES        # Must be "NO" or "YES" (uppercase)
DNAME=aidpd       # daemon name
DPATH=/bin        # daemon path
ShellInit=""      # Performs any required initialization
ConfigFiles=""    # configuration files
DTYPE=cmd         # must be "sig" or "cmd" (lowercase)
DSIG=             # signal to stop/restart the daemon (lowercase)
                  # if it's hup term will be used to stop the daemon
# daemon command line parameters
DPARM=""
DSTOP="-k"
