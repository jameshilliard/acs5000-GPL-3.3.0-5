# This file defines the pmd configuration

ENABLE=YES        # Must be "NO" or "YES" (uppercase)
DNAME=pmd_ng         # daemon name
DPATH=/bin        # daemon path
# Performs any required initialization
ShellInit=""
# configuration files
ConfigFiles="/etc/pmd.conf /etc/pmd.users /etc/pmdgrp.conf"
DTYPE=sig         # must be "sig" or "cmd" (lowercase)
DSIG=hup          # signal to stop/restart the daemon (lowercase)
                  # if it's hup term will be used to stop the daemon
WAITK=YES         # Waits the process finish before starts again
PIDF=/var/run/pmdng.pid # PMD PID file
# daemon command line parameters
DPARM=""
DSTOP=
