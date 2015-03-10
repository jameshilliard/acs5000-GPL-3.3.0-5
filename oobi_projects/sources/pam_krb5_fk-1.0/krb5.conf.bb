[logging]
 default = FILE:/var/log/krb5libs.log
 kdc = FILE:/var/log/krb5kdc.log
 admin_server = FILE:/var/log/kadmind.log

[libdefaults]
 ticket_lifetime = 24000
 default_realm = blackbox.com
 default_tgs_enctypes = des-cbc-crc
 default_tkt_enctypes = des-cbc-crc

[realms]
 blackbox.com= {
  kdc = 64.186.161.125:88
  admin_server = 61.186.161.125:749
  default_domain = blackbox.com
 }

[domain_realm]
 .blackbox.com = blackbox.com
 blackbox.com = blackbox.com

[kdc]
 profile = /var/kerberos/krb5kdc/kdc.conf

[pam]
 debug = true
 ticket_lifetime = 36000
 renew_lifetime = 36000
 forwardable = true
 krb4_convert = true
