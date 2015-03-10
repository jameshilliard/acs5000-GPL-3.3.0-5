/*-----------------------------------------------------------------------------
 *	Copyright (C) Cyclades Corporation, 1997.
 *	All rights reserved
 *
 *	PR-3000 Router
 *
 *	This file:	flash.h
 *	Description:	This file contains the definitions and the prototypes 
 *			related to the flash module
 *	
 *	Complies with Cyclades SW Coding Standard rev 1.2.
 *-----------------------------------------------------------------------------
 */
 
/*-----------------------------------------------------------------------------
 *	Change History
 *-----------------------------------------------------------------------------
 * 04/25/96	V 1.0.0		Elio Lerner
 *	First Release
 * 04/23/96	V 1.0.1		Helio Fujimoto
 *	Port to PR-3000 for test
 * 07/28/96	V 1.0.2		Helio Fujimoto
 *	Port to PR-3000
 *-----------------------------------------------------------------------------
 */

/*--------------------------------------------------------------------------
 *
 * Constants
 *
 *-------------------------------------------------------------------------*/

/* Values used on dealing with the flash */
/* The values are flipped because the data lines on the PR-3000 were flipped */
#define UNLOCK_VALUE_A				0xaa
#define UNLOCK_VALUE_B				0x55
#define SETUP_VALUE					0x80
#define	SECTOR_ERASE_VALUE			0x30
#define	CHIP_ERASE_VALUE			0x10
#define PROGRAM_VALUE				0xa0
#define RESET_READ_VALUE			0xf0
#define AUTO_SELECT_VALUE			0x90
#define	DQ7							0x80
#define DQ6							0x40
#define DQ5							0x20		
#define DQ3							0x08		

/* Configuration Address */
#define CFG_ADDR					0x5555

/* values returned by the data_polling procedure */
#define PASS			0x01
#define FAIL			0x00
#define IN_PROGRESS		0x03

typedef struct flash_config_vector_header {
    unsigned short type;
    unsigned short size;
    unsigned short crc;
	unsigned char  data[2];
} FLASH_CONFIG_VECTOR_HEADER;

#define FLASH_ADDR ((unsigned char *)(((immap_t *)IMAP_ADDR)->im_memctl.memc_br0 & 0xffff8000L))

extern unsigned char *cflash_addr, *flash_addr, *flash_boot;

