/*----------------------------------------------------------------------*/
/*      Copyright (C) 1996 - CYCLADES Corporation.                      */
/*                                                                      */
/*      Module: CONFIG.H                                                */
/*                                                                      */
/*      Version Date     Author            Comment                      */
/*                                                                      */
/*      V_1.0.0 08/25/00 Edson A. Seabra   Module creation.             */
/*                                                                      */
/*----------------------------------------------------------------------*/

#define IP_ALEN	4
#define EP_ALEN	6
#define PATH_LEN 40
#define FLASH_SIGN_SIZE 8

struct type0000 {
    unsigned char    flash_sign[FLASH_SIGN_SIZE];    /* mark initialized CMOS */
    unsigned char    version;            /* Configure vector version */
    unsigned char    routing_protocol;   /* RIP, OSPF, etc */
    unsigned char    save_sw;            /* TRUE : the RTBOOT must be saved into flash */
};

struct HW_SETUP {
    unsigned char    boot_option;
    unsigned char    tftp_path[PATH_LEN];  /* path and filename for tftp during boot */
    unsigned char    enable_wdt;           /* if true then Watch Dog Timer enabled */
    unsigned char    op_code_crc;
    unsigned char    mem_test;
    unsigned int     ibytesec; /*maximum ehternet rate limit*/
    unsigned short   max_int_work;
    unsigned char    physical_addr[EP_ALEN];
    unsigned char    itf_ip_addr[IP_ALEN];
    unsigned char    itf_subnet_mask[IP_ALEN];
    unsigned char    console_speed;
    unsigned char    lcd_reboot;
    unsigned char    cpu_utilization;
    unsigned char    mii_operation;
};

struct BOOT_MASK {
    unsigned char boot_type;    /* It can assume Bootp, TFTP or Both */
    unsigned char intfc;        /* Interface where the boot will happen */
    unsigned char bootserver[IP_ALEN];    /* Server IP address */
};

typedef struct _OM_INFO {
	unsigned char mac_addr[6];
	unsigned char banner[100];
	unsigned char serial_number[5];
	unsigned char reserved[135];
} OM_INFO;

#define TYPE0000    0
#define TYPE0015    15
#define TYPE0068    68
#define TYPE9000    9000

#define OM_ADDRESS			(1024 - 6 - sizeof(OM_INFO))

#define OM_ADDRESS1			(0x10000L - 1024 + OM_ADDRESS)
#define OM_ADDRESS2			(0x20000L - 1024 + OM_ADDRESS)

#define CNF_ADDRESS1		0x30000
#define CNF_ADDRESS2		0x20000

