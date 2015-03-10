#!/bin/sh

TOOLS=$(dirname $0)

. ${TOOLS}/shell_utils

#
# Start
#

check_vars ROOT_PROJ TARGET_DIR SOURCE_DIR

check_dirs $ROOT_PROJ $SOURCE_DIR

install -d $TARGET_DIR

: ${COPY_HDR:=no}

cd $SOURCE_DIR

LNMODE=-fs

for file in $(find -true | grep -v CVS ); do
	if [ -L $file ]; then
        if [ "$COPY_HDR" = "yes" -a "${file:(-2)}" = ".h" ] ; then
          cp $SOURCE_DIR/$file $TARGET_DIR/$file || exit -1
        else
		  ln $LNMODE $(readlink $SOURCE_DIR/$file) $TARGET_DIR/$file
        fi
   	elif [ -d $file ]; then
		[ -d $TARGET_DIR/$file ] || (install -d $TARGET_DIR/$file || exit -1)
	else
        if [ "$COPY_HDR" = "yes" -a "${file:(-2)}" = ".h" ] ; then
          cp $SOURCE_DIR/$file $TARGET_DIR/$file || exit -1
        else
		  ln $LNMODE $SOURCE_DIR/$file $TARGET_DIR/$file
        fi
	fi
done

exit 0

