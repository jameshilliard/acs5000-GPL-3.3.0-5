
/*
 * A collection of structures, addresses, and values associated with
 * the PRxK board.  Copied from the EST8xx stuff.
 *
 * Copyright (c) 1998 Dan Malek (dmalek@jlc.net)
 */
#ifndef __MACH_PRxK_DEFS
#define __MACH_PRxK_DEFS

#ifndef __ASSEMBLY__
#define CYC_BOARD_TYPE (((bd_t *)__res)->hw_info.board_type)

#define BOARD_PR3K 0
#define BOARD_PR2K 1
#define BOARD_PR4K 2
#define BOARD_PR1K 3
#define BOARD_TS1K 4
#define BOARD_TS2K 5
#define BOARD_TS4H 6
#define BOARD_TS8H 7
#define BOARD_TS3K 8
#define BOARD_TX1K 9
#define BOARD_TX2K 10
#define BOARD_PL1K 11
#define BOARD_TS1H 12
#define BOARD_NL1K 13
#define BOARD_TS1100 14
#define BOARD_TS2100 15
#define BOARD_SM100  16
#define BOARD_PR3500 17
#define BOARD_TS110  18
#define BOARD_ACS48  19
#define BOARD_ACS4   20
#define BOARD_ACS8   21
#define BOARD_ACS1   22
#define BOARD_KVM16  24
#define BOARD_KVM32  25
#define BOARD_KVMNET16  26
#define BOARD_KVMNET32  27
#define BOARD_ONS441	28
#define BOARD_ONS481	29
#define BOARD_ONS841	30	
#define BOARD_ONS881	31
#define BOARD_ONS442	32	
#define BOARD_ONS482	33	
#define BOARD_ONS842	34	
#define BOARD_ONS882	35
#define BOARD_KVMP16	36
#define BOARD_KVMP32	37

///* IDE settings */
//#define IDE0_BASE_OFFSET		0
//#define IDE0_DATA_REG_OFFSET		0
//#define IDE0_ERROR_REG_OFFSET		3
//#define IDE0_NSECTOR_REG_OFFSET		5
//#define IDE0_SECTOR_REG_OFFSET		7
//#define IDE0_LCYL_REG_OFFSET		9
//#define IDE0_HCYL_REG_OFFSET		11
//#define IDE0_SELECT_REG_OFFSET		13
//#define IDE0_STATUS_REG_OFFSET		15
//#define IDE0_CONTROL_REG_OFFSET		0x1D
//#define IDE0_IRQ_REG_OFFSET		0x1F	/* not used */
//
//#define	IDE0_INTERRUPT			5

/* PRxB boot code information data,
 * they come from FPGA and Dip switchs
*/
struct	prxk_boot_info {
	unsigned int	mem_test_result;
	unsigned int	memory_detected;
	unsigned char	alt_boot;
	unsigned char	error_status;
	unsigned char	reserved;
	unsigned char	board_type;
	unsigned int	flash_size;
	unsigned char	clock_freq;
	unsigned char	processor;
	unsigned char	manufactor;
	unsigned char	eth_links;
	unsigned char	board_version;
	unsigned char	save_opcode;
	unsigned char	code_legacy;
	unsigned char	flash_chips;
	unsigned char	serial_itf;
	unsigned char	power_supply;
	unsigned char	pcmcia;
	unsigned char	wdt_config;
	unsigned char	rtc_value[20];
	unsigned int	storage_size;	//[RK] compact flash size
	unsigned char	device_id[16];	//[RK] router id
	unsigned char	async_config;	// KVMNET USER2 profile
	unsigned char	fpga_version;	// FPGA version
	unsigned char	ipboard1;
	unsigned char	ipboard2;
	unsigned char	next_available;
};

/* A Board Information structure that is given to a program when
 * prom starts it up.
 */
typedef struct bd_info {
#ifdef CONFIG_UBOOT
        unsigned long   bi_memstart;    /* start of DRAM memory */
        unsigned long   bi_memsize;     /* size  of DRAM memory in bytes */
        unsigned long   bi_flashstart;  /* start of FLASH memory */
        unsigned long   bi_flashsize;   /* size  of FLASH memory */
        unsigned long   bi_flashoffset; /* reserved area for startup monitor */
        unsigned long   bi_sramstart;   /* start of SRAM memory */
        unsigned long   bi_sramsize;    /* size  of SRAM memory */
        unsigned long   bi_immr_base;   /* base of IMMR register */
        unsigned long   bi_bootflags;   /* boot / reboot flag (for LynxOS) */
        unsigned long   bi_ip_addr;     /* IP Address */
        unsigned char   bi_enetaddr[6]; /* Ethernet adress */
        unsigned short  bi_ethspeed;    /* Ethernet speed in Mbps */
        unsigned long   bi_intfreq;     /* Internal Freq, in MHz */
        unsigned long   bi_busfreq;     /* Bus Freq, in MHz */
	unsigned int	bi_baudrate;
	unsigned char	wdt;
	unsigned char	mii_operation;
#else
	unsigned int	bi_memstart;	/* Memory start address */
	unsigned int	bi_memsize;	/* Memory (end) size in bytes */
	unsigned int	bi_intfreq;	/* Internal Freq, in Hz */
	unsigned int	bi_busfreq;	/* Bus Freq, in Hz */
	unsigned char	wdt;
	unsigned char	mii_operation;
	unsigned char	bi_enetaddr[6];
	unsigned int	bi_baudrate;
#endif
	unsigned short	max_int_work;
	unsigned short	num_portsS;
	unsigned short	num_portsA;
	unsigned short	num_portsM;
	unsigned short	num_portsK;
	unsigned char	board_name[32];
	unsigned char	vendor_name[32];
	unsigned char	prod_line[32];
	unsigned char	prod_name[32];
	unsigned char	card_name[10];
	unsigned char   cyc_banner[256];
	unsigned char   cyc_version[32];
	struct	prxk_boot_info hw_info;
	unsigned int 	ibytesec; /*receive rate limit*/	
} bd_t;

/* Memory map is configured by the PROM startup.
 * We just map a few things we need.
 */
#define IMAP_ADDR		((uint)0xff000000)
#define IMAP_SIZE		((uint)(64 * 1024))

#endif  //__ASSEMBLY__
/* We don't use the 8259.
*/
#define NR_8259_INTS	0

#define FEC_INTERRUPT   SIU_LEVEL5


/* Machine type
*/
#define _MACH_8xx (_MACH_PRxK)

#endif
