# This file defines Generic Dial Framework start/stop configuration

ENABLE=NO          # Must be "NO" or "YES" (uppercase)
DNAME=generic-dial # daemon name
DPATH=/bin         # daemon path
ShellInit=         # Performs any required initialization
ConfigFiles=/etc/generic-dial.conf # configuration files
DTYPE=sig          # must be "sig" or "cmd" (lowercase)
DSIG=term          # signal to stop/restart the daemon (lowercase)
                   # if it's hup, term will be used to stop the daemon
# daemon command line parameters
DPARM="instance-identification" # the application adds instance id to process name
DSTOP=
