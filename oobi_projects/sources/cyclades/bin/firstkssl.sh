#!/bin/bash

SSLBIN=/bin/openssl
DAYS=365
RSAKEY=1024
CONFSBJ=/etc/req_key
export RANDFILE=/tmp/.rnd

#Check directory /etc/CA
if [ -d /etc/CA ]
then
    KEYDIR=/etc/CA
else
    echo "[$0] - Directory /etc/CA not found."
    exit 1
fi

#Check if it is the first time
if [ ! -f ${KEYDIR}/server.pem ] || [ ! -f ${KEYDIR}/cert.pem ]
then
    echo "Generating SSLkey:" 1> /dev/null 2>&1

    if [ -f ${CONFSBJ} ]
    then
        #Create private key temp and cert file
        if [ -x ${SSLBIN} ]
        then
            ${SSLBIN} req -x509 -nodes -days ${DAYS} -config ${CONFSBJ} -newkey rsa:${RSAKEY} -keyout /tmp/mycert.pem -out /tmp/mycert1.pem 1> /dev/null 2>&1
        else
            echo "[$0] - File ${SSLBIN} not found."
            exit 2
        fi

        cat /tmp/mycert1.pem >> /tmp/mycert.pem
        mv /tmp/mycert.pem ${KEYDIR}/server.pem
        mv /tmp/mycert1.pem ${KEYDIR}/cert.pem

        if [ -x /bin/updatefiles ]
        then
            for cf_file in server.pem cert.pem
            do
                grep "${KEYDIR}/${cf_file}" /etc/config_files 1> /dev/null 2>&1
                if [ $? -eq 1 ]
                then
                    echo "${KEYDIR}/${cf_file}" >> /etc/config_files
                fi
            done

            /bin/updatefiles ${KEYDIR}/* 1> /dev/null 2>&1
        else
            echo "[$0] - WARNING: The updatefiles command not found."
        fi
    else
        echo "[$0] - File ${CONFSBJ} not found."
    fi
fi
