#! /bin/sh 

CARD=`cardctl ident | grep -c XE2000 `
if [ "$CARD" = "0" ];then
        OPT=0;
else
        OPT=4
fi
modprobe --ignore-install xirc2ps_cs if_port=$OPT
