# This file defines the event_gen configuration

ENABLE=YES        # Must be "NO" or "YES" (uppercase)
DNAME=event_gen   # daemon name
DPATH=/bin       # daemon path
ShellInit="/bin/create_evt_pipe.sh"        # Performs any required initialization
ConfigFiles="/etc/event_notif.conf /etc/event_notif.lnk"      # configuration files
DTYPE=sig         # must be "sig" or "cmd" (lowercase)
DSIG=hup          # signal to stop/restart the daemon (lowercase)
                  # if it's hup term will be used to stop the daemon
# daemon command line parameters
DPARM=""
DSTOP=
