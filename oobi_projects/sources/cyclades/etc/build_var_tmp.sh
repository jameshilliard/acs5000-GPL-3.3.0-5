#!/bin/sh

tmp_size=20000000
var_size=5000000

mount -t tmpfs none  /var -o size=$var_size >/dev/null 2>&1
mount -t tmpfs none  /tmp -o size=$tmp_size >/dev/null 2>&1

