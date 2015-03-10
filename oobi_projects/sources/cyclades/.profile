# .profile

#echo Exec "/.profile" Dir `pwd` 

# Get the aliases and functions
if [ -f ~/.bashrc ]; then
        . ~/.bashrc
fi

# User specific environment and startup programs

EDITOR=/bin/vi

if [ "x`tty`" = "x/dev/ttyS0" ] ; then
  TERM='vt100'
else
  TERM='linux'
fi

USERNAME="root"
PATH=$PATH:/usr/local/sbin:/usr/sbin:/sbin

export USERNAME TERM PATH EDITOR

