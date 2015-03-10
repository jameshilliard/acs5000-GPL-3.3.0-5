# This file defines the DB start/stop configuration

ENABLE=YES           # Must be always yes
DNAME=cy_buffering   # daemon name
DPATH=/bin           # daemon path
FS_LOC=CF            # valid values CF or LOCAL
# Performs any required initialization
ShellInit="/bin/create_DB_link $FS_LOC"
# configuration files
ConfigFiles=/bin/create_DB_link
DTYPE=sig            # must be "sig" or "cmd" (lowercase)
DSIG=int             # signal to stop/restart the daemon (lowercase)
                     # if it's hup term will be used to stop the daemon
# daemon command line parameters
DPARM=""
DSTOP=
