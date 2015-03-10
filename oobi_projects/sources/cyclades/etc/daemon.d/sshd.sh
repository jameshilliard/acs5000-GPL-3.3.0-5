# This file defines the ssh service configuration

ENABLE=YES        # Must be "NO" or "YES" (uppercase)
DNAME=sshd        # daemon name
DPATH=/bin        # daemon path
ShellInit=/bin/firstkssh.sh # Performs any required initialization
# configuration files
ConfigFiles=/etc/ssh/sshd_config
DTYPE=sig         # must be "sig" or "cmd" (lowercase)
DSIG=hup          # signal to stop/restart the daemon (lowercase)
                  # if it's hup term will be used to stop the daemon
PIDF=/var/run/sshd.pid
# daemon command line parameters
DPARM="-f /etc/ssh/sshd_config"
DSTOP=
