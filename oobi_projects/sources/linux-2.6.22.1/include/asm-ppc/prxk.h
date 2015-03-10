
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
#define BOARD_AVCS1	38
#define BOARD_AVCS4	39
#define BOARD_AVCS8	40
#define BOARD_AVCS16	41
#define BOARD_AVCS32	42
#define BOARD_AVCS48	43
#define BOARD_ACS5001   44
#define BOARD_ACS5004   45
#define BOARD_ACS5008   46
#define BOARD_ACS5016   47
#define BOARD_ACS5032   48
#define BOARD_ACS5048   49



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
	unsigned int	bi_memstart;	/* Memory start address */
	unsigned int	bi_memsize;	/* Memory (end) size in bytes */
	unsigned int	bi_intfreq;	/* Internal Freq, in Hz */
	unsigned int	bi_busfreq;	/* Bus Freq, in Hz */
	unsigned char	wdt;
	unsigned char	mii_operation;
	unsigned char	bi_enetaddr[6];
	unsigned int	bi_baudrate;
	unsigned int    ibytesec;  //[GB]May/06/05  Ethernet Receive Rate Limit
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
	unsigned char   cyc_version[48]; //[RK]Dec/07/05 
	unsigned char   dsview_plugin[10]; //[RK]Jun/07/06 
	struct	prxk_boot_info hw_info;
	unsigned char   cyc_serial_number[5]; //[RK]Jul/01/10
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
