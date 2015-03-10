#!/bin/sh

. /etc/billing_up.conf

TEMP_FILE="/tmp/billing_up_cmd"
LOG_FILE="/tmp/billing_log"

CWD=`pwd`
cd $LOCAL_DIRECTORY

ls *.txt >>/dev/null 2>&1 || exit 0

echo "user $USER $PASSWORD" > $TEMP_FILE
echo "bin" >> $TEMP_FILE
[ -n "$REMOTE_DIRECTORY" ] && echo "cd $REMOTE_DIRECTORY" >> $TEMP_FILE
FILES=`ls *.txt 2>/dev/null`
echo "$FILES" | while read line; do
	echo "put $line" >> $TEMP_FILE
done
ftp -inv $REMOTE_SERVER > $LOG_FILE < $TEMP_FILE

if grep "Not connected" $LOG_FILE > /dev/null 2>&1; then
	exit 0
else
   cat $LOG_FILE |
   awk -v FILES_DIR=$LOCAL_DIRECTORY '
   BEGIN{
   COMMAND_OK=200
   SEND_OK=150
   RESULT_OK=226
   }
   {
      if( $1 ~/local:/ ) {
         FILE=$2
         getline
         if( $1 != COMMAND_OK )
            system("echo "FILE" transfer failed.\n")
         else {
            getline
            if( $1 != SEND_OK )
               system("echo "FILE" transfer failed.\n")
            else {
               getline
               if( $1 != RESULT_OK )
                  system("echo "FILE" transfer failed.\n")
               else {
                  system("rm -f "FILES_DIR"/"FILE" 2>/dev/null")
               }
            }
         }
      }
   }'
fi
rm $TEMP_FILE $LOG_FILE >/dev/null 2>&1

cd $CWD
