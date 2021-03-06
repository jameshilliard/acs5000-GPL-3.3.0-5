# .bashrc

#echo Exec "/.bashrc" Dir `pwd` 

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

if [ ! -e /etc/security.opts ]; then
    more /sec-warning
fi

if [ "/dev/ttyS0" = "`tty`" ] && [ -e /etc/security.opts ]; then
  # display useful information to root user
  if [ -f /readme ]; then
    version="`set_nports 9`."
    prod="`set_nports 6`-`set_nports 7`"
    echo Welcome to $prod $version
    more /readme
  fi
fi
