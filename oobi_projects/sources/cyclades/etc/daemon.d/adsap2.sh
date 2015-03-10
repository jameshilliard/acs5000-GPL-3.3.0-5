# This file defines the adsap2 configuration

ENABLE=YES        # Must be "NO" or "YES" (uppercase)
DNAME=adsap2      # daemon name
DPATH=/bin        # daemon path
ShellInit="mkdir -p /etc/CA/adsap2"  # Performs any required initialization
ConfigFiles="/etc/adsap2.conf"      # configuration files
DTYPE=sig         # must be "sig" or "cmd" (lowercase)
DSIG=hup          # signal to stop/restart the daemon (lowercase)
                  # if it's hup term will be used to stop the daemon
# daemon command line parameters
DPARM=""
DSTOP=
