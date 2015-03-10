#!/bin/sh

declare BASENAME

if [ -z "$1"  -o ! -f ${1}.list ] ; then
  echo "*"
  echo "* ERROR - USAGE: $0 file.list"
  echo "*"
  exit
fi

BASENAME=$1
rm -rf ${BASENAME}/*
mkdir -p ${BASENAME}
exec > ${BASENAME}.sh

echo "#!/bin/sh"
echo
echo "BASENAME=${BASENAME}"
echo
echo "mount -o remount,rw /dev/root"
echo
echo "cd \${BASENAME}"
echo "cp -a * /"
echo
echo "mount -o remount,ro /dev/root"
echo

exec >&2

while read localname remotename
do
  if [ -f $localname ] ; then
    echo Including file: $remotename
    install -D $localname ${BASENAME}/$remotename
  else
    echo "*" >&2
    echo "* Error - file '$localname' does not exist" >&2
    echo "*" >&2
    exit 1
  fi
done << --EOF
$(<${BASENAME}.list)
--EOF

chmod +x ${BASENAME}.sh
tar -czf ${BASENAME}.gz ${BASENAME} ${BASENAME}.sh
md5sum ${BASENAME}.gz > ${BASENAME}.gz.md5

