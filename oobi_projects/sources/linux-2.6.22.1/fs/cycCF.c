#include <asm/mpc8xx.h>
#include <asm/8xx_immap.h>
#include <asm/cpld.h>

/* structure */
typedef struct sCF {
	unsigned short data;
	unsigned char  dummy1;
	unsigned char  error_and_features;
	unsigned char  dummy2;
	unsigned char  sec_cnt;
	unsigned char  dummy3;
	unsigned char  sec_num;
	unsigned char  dummy4;
	unsigned char  cyl_low;
	unsigned char  dummy5;
	unsigned char  cyl_high;
	unsigned char  dummy6;
	unsigned char  sel_card_head;
	unsigned char  dummy7;
	unsigned char  status_and_command;
	unsigned char  dummy8[0xd];
	unsigned char  alt_status_and_dev_ctl;
	unsigned char  dummy9;
	unsigned char  drv_card_addr;
} CF;

/* static variables */
static volatile CF *cfs;
static char dummy_status;
static unsigned short cf_buf[256];
static int max_lba_addr;

/* CF Status and Alternate Status Register */
#define CF_STATUS_BUSY		0x80
#define CF_STATUS_RDY		0x40
#define CF_STATUS_DSC		0x10
#define CF_STATUS_DRQ		0x08
#define CF_STATUS_ERROR		0x01

/* CF Device Control Register */
#define CF_INT			0x02
#define CF_INT_ENABLE		0x00

/* CF Drive/Head Register */
#define CF_LBA			0x40

/* FPGA MISC Bit Map */
#ifdef CONFIG_TSxK
#define CF_CARD_DETECT_2	0x80
#define CF_CARD_DETECT_1	0x40
#define CF_RESET		0x02
#define CF_MISC_INT		0x04
#endif

#define CF_NLOOP		100000		
#define CF_DELAY		100		/* in usec */
#define CF_CMD_DELAY		30000	/* in usec */	
#define CF_CLASS_MASK		0xff00
#define CF_CMD_MASK		0x00ff
#define CF_CLASS_1		0x0100
#define CF_CLASS_2		0x0200
#define CF_CLASS_3		0x0300
#define CF_SIGNATURE		0x848a	/* CF general configuration signature */
#define IDE_SIGNATURE	   0x45A	/* IDE HD general configuration signature */
#define CF_CLS			0x1	/* clear status register */
#define CF_NO_CLS		0x0	/* no clear status register */
#define CF_LBA_MASK		0x200	/* LBA capability support */

/* CF Class and Code of Command Set */
#define CF_CMD_CHECK_POWER_MODE		(CF_CLASS_1 | 0x98)		/* check power mode */
#define CF_CMD_EXE_DRV_DIAG		(CF_CLASS_1 | 0x90)		/* execute driver diagnostic */
#define CF_CMD_ID_DRV			(CF_CLASS_1 | 0xEC)		/* identify driver */
#define CF_CMD_RD_SECTOR		(CF_CLASS_1 | 0x20)		/* read sectors */
#define CF_CMD_WR_SECTOR		(CF_CLASS_2 | 0x30)		/* write sectors */
#define CF_IDLE				(CF_CLASS_1 | 0xE3)		/* put CF in idle mode */
#define CF_CMD_ERASE_SECTOR		(CF_CLASS_1 | 0xC0)		/* erase sectors */
#define CF_CMD_RDM_SECTOR		(CF_CLASS_1 | 0xC4)		/* read multiple sectors */
#define CF_CMD_MULTIPLE_MODE		(CF_CLASS_1 | 0xC6)		/* multiple mode */
#define CF_CMD_WR_ERASED_SECTOR		(CF_CLASS_2 | 0x38)		/* write sectors w/o erase */

/* CF Command Response */
#define CF_ST_POWER_MODE_OK		0xFF	/* status power mode ok */
#define CF_ST_NO_ERR_DET		0x01	/* status no error detected */

/* CF Identify Driver Mask */
#define CF_GEN_INFO			0		/* general info signature, 2 bytes */
#define CF_VENDOR_ID			9		/* vendor ID, 2 bytes */
#define CF_SN				10		/* serial number, 20 bytes */
#define CF_MODEL_NUM			27		/* model number, 40 bytes */
#define CF_CAPABILITIES			49		/* capabilities */
#define CF_MAX_LBA			60		/* max num of sectors in LBA mode, 4 bytes */

/* CF Power mode */
#define CF_POWER_DOWN_MODE_DIS		0		/* power down mode disable */

// get_dec_timer - gets the decrementer timer register value
static int get_dec_timer(void)
{
	int ret;
	asm("mfspr	%0,22\n" : "=r" (ret));
	return ret;
}

#define INITIAL_VALUE   0x270F

static int flag_ide_init = 1;

static int calc_delay(int start_real_time, int start_dec_time)
{
	int end_dec_time, delta, delta_ticks;

	end_dec_time = get_dec_timer();
	delta_ticks = jiffies - start_real_time;
	if (delta_ticks == 0) {
		delta = start_dec_time - end_dec_time;
	} else {
		delta_ticks--;
		delta = start_dec_time + INITIAL_VALUE - end_dec_time;
	}
	if (delta < 0) {
		delta = 0;
	}
	return(delta_ticks * (INITIAL_VALUE + 1) + delta);
}

// this routine performs a delay in microseconds.
static void delay_us (int usec)
{
	int delay, start_real_time, start_dec_time;

	start_real_time = jiffies;
	start_dec_time = get_dec_timer();
	delay = 0;
	while (delay < usec){
		if (!flag_ide_init) {
			schedule();
		}
		delay = calc_delay(start_real_time, start_dec_time);
	}
}

static int cyc_cf_check_status(int usec, unsigned char bits, int flag)
{
	int delay, start_real_time, start_dec_time;

	start_real_time = jiffies;
	start_dec_time = get_dec_timer();
	delay = 0;
	while (1){
		if (flag) {
			if  ((cfs->status_and_command & bits)) {
				return(1);
			}
		} else {
			if  (!(cfs->status_and_command & bits)) {
				return(1);
			}
		}
		if (delay >= usec) break;
		if (!flag_ide_init) {
			schedule();
		}
		delay = calc_delay(start_real_time, start_dec_time);
	}
	return 0;
}

// * this routine sends command to compact flash and return only 
// * when command is done.
static int cyc_cf_send_cmd(int command)
{
	if (!cyc_cf_check_status(CF_NLOOP * CF_DELAY, CF_STATUS_BUSY, 0)) {
		/* timeout waiting CF get ready */
		return 0;
	}

	cfs->status_and_command = (command & CF_CMD_MASK);

	if ((command & CF_CLASS_MASK) > CF_CLASS_2) {
		delay_us(CF_CMD_DELAY);
	}
	else {
		delay_us(CF_DELAY);
	}

	if (!cyc_cf_check_status(CF_NLOOP * CF_DELAY, CF_STATUS_BUSY, 0)) {
		/* timeout waiting CF get ready */
		return 0;
	}

	/* clear interrupt pending status */
	if (cfs->status_and_command & CF_STATUS_ERROR) {
		return 0;
	}	
	return 1;
}

// * this routine detects and program compact flash 
static int cyc_cf_detect (void) 
{
	int loop, i;
	int warm_up;
	bd_t	*bd = (bd_t *)__res;

	flag_ide_init = 1;
	max_lba_addr = 0;

	cfs = (CF *)cflash_addr;

#ifdef CONFIG_TSxK
	if (bd->hw_info.board_type == BOARD_ACS48 || 
		bd->hw_info.board_type == BOARD_AVCS48 ||
		bd->hw_info.board_type == BOARD_ACS5048) {
		cpld->fpga_led_ctl |= 0x08;
	}

	if (cpld->cpld_misc & (CF_CARD_DETECT_1|CF_CARD_DETECT_2)) {
		/* IDE HD present */
		warm_up = 1000000;	/* 1s warm up delay */
	} else {
		/* compact flash present */
		warm_up = 200000;	/*  200ms warm up delay */
	}
	cpld->cpld_misc |= CF_RESET;	/* compact flash reset */
	delay_us(500000);		/* 500ms */
	cpld->cpld_misc &= ~CF_RESET;	/* compact flash released */
	delay_us(warm_up);

#endif /*CONFIG_TSxK*/

	loop = CF_NLOOP;
	while ((cfs->alt_status_and_dev_ctl & (CF_STATUS_BUSY|CF_STATUS_DSC|CF_STATUS_RDY)) != (CF_STATUS_DSC|CF_STATUS_RDY)) {
		loop--;
		if (!loop) {
			/* timeout waiting CF get ready */
			return 0;
		}
		delay_us(CF_DELAY);
	}
	dummy_status = cfs->status_and_command;	/* clear status */

	/* all following access are for Driver #0 */

	cfs->sec_cnt = CF_POWER_DOWN_MODE_DIS;
	/* set power mode idle */
	if (!cyc_cf_send_cmd(CF_IDLE)) {
		return 0;	/* command failed */
	}

	/* check power mode */
	if (!cyc_cf_send_cmd(CF_CMD_CHECK_POWER_MODE)) {
		return 0;	/* command failed */
	}
	if (cfs->sec_cnt != CF_ST_POWER_MODE_OK) {
		return 0;	/* power mode not ok */
	}

	/* execute drive diagnostic */
	if (!cyc_cf_send_cmd(CF_CMD_EXE_DRV_DIAG)) {
		return 0;	/* command failed */
	}
	if (cfs->error_and_features != CF_ST_NO_ERR_DET) {
		return 0;	/* diagnostic not ok */
	}

	/* identify drive */
	if (!cyc_cf_send_cmd(CF_CMD_ID_DRV)) {
		return 0;	/* command failed */
	}
	for (i=0; i<256; i++) {
		cf_buf[i] = cfs->data;		/* read identification */
	}

	if (!(cf_buf[CF_CAPABILITIES] & CF_LBA_MASK)) {
		return 0; 	/* no LBA capabilities */
	}

	max_lba_addr = cf_buf[CF_MAX_LBA] + (cf_buf[CF_MAX_LBA+1] << 16);

	flag_ide_init = 0;
	return 1;
}

// * this routine performs a read. Buffer address must have  
// * alignment 2 and size must be multiple of 512 bytes.
static int cyc_cf_read (unsigned char *buf, int lba_addr, int size) 
{
	int fifo_size, n_sectors, m_sectors;
	unsigned short *buffer;

	if ((size % 512)  || (((int)buf) % 2)) {
		return 0;
	}
	
	/* set variables to word mode */
	buffer = (unsigned short*) buf;
	n_sectors = size / 512;

	if ((n_sectors + lba_addr) > max_lba_addr) {
		return 0;	/* LBA address overflow */
	} 

	while (n_sectors) {
		if (!cyc_cf_check_status(CF_NLOOP * CF_DELAY, CF_STATUS_BUSY, 0)) {
			/* timeout waiting CF get ready */
			return(0);
		}
		/* set LBA address and LBA mode */
		cfs->sec_num = lba_addr & 0xff;
		cfs->cyl_low = (lba_addr >> 8) & 0xff;
		cfs->cyl_high = (lba_addr >> 16) & 0xff;
		cfs->sel_card_head &= ~0x0f;		/* erase HS3-HS0 */
		cfs->sel_card_head |= (((lba_addr >> 24) & 0x0f) | CF_LBA);
		/* read sectors */
		if (n_sectors >= 256) {
			m_sectors = 256;
			cfs->sec_cnt = 0;
		} else {
			m_sectors = n_sectors;
			cfs->sec_cnt = n_sectors;
		}
		/* send command to read */
		if (!cyc_cf_send_cmd(CF_CMD_RD_SECTOR)) {
			return 0; 		/* command failed */
		}
		/* next loop */
		lba_addr += m_sectors;
		n_sectors -= m_sectors;
		while (m_sectors--) {
			/* wait for NOT BUSY */
			if (!cyc_cf_check_status(CF_NLOOP * CF_DELAY, CF_STATUS_DRQ, 1)) {
				/* timeout waiting CF get ready */
				return 0;
			}
			/* read 256 words */
			fifo_size = 256;
			while (fifo_size--) {
				*buffer++ = cfs->data;			
			}
			delay_us(CF_DELAY);
		}
	}
	return 1;
}

// *  this routine performs a write. Buffer address must have
// *   alignment 2 and size must be multiple of 512 bytes.
static int cyc_cf_write (unsigned char *buf, int lba_addr, int size) 
{
	int fifo_size, n_sectors, m_sectors, lba, cmd;
	unsigned short *buffer;

	if ((size % 512)  || (((int)buf) % 2)) {
		return 0;
	}

	/* set variables */
	buffer = (unsigned short*) buf;
	n_sectors = size / 512;
	lba = lba_addr;

	/* check if there is room */
	if ((n_sectors + lba_addr) > max_lba_addr) {
		return 0;	/* LBA address overflow */
	} 

	if (cf_buf[CF_GEN_INFO] == CF_SIGNATURE) {
		if (!cyc_cf_check_status(CF_NLOOP * CF_DELAY, CF_STATUS_BUSY, 0)) {
			/* timeout waiting CF get ready */
			return(0);
		}
		/* compact flash device */
		while (n_sectors) {
			/* set LBA address and LBA mode */
			cfs->sec_num = (lba & 0xff);
			cfs->cyl_low = (lba >> 8) & 0xff;
			cfs->cyl_high = (lba >> 16) & 0xff;
			cfs->sel_card_head &= ~0x0f;			/* erase HS3-HS0 */
			cfs->sel_card_head |= (((lba >> 24) & 0x0f) | CF_LBA);
			/* erase multiples sectors */
			if (n_sectors >= 256) {
				m_sectors = 256;
				cfs->sec_cnt = 0;
			} else {
				m_sectors = n_sectors;
				cfs->sec_cnt = n_sectors;
			}
			/* send command to erase */
			if (!cyc_cf_send_cmd(CF_CMD_ERASE_SECTOR)) {
				return 0; 			/* command failed */
			}
			/* next loop */
			lba += m_sectors;
			n_sectors -= m_sectors;
		}
		cmd = CF_CMD_WR_ERASED_SECTOR;
	} else {
		/* HD device */
		cmd = CF_CMD_WR_SECTOR;
	}
	
	n_sectors = size / 512;
	while (n_sectors) {
		if (!cyc_cf_check_status(CF_NLOOP * CF_DELAY, CF_STATUS_BUSY, 0)) {
			/* timeout waiting CF get ready */
			return(0);
		}
		/* set LBA address and LBA mode */
		cfs->sec_num = (lba_addr & 0xff);
		cfs->cyl_low = (lba_addr >> 8) & 0xff;
		cfs->cyl_high = (lba_addr >> 16) & 0xff;
		cfs->sel_card_head &= ~0x0f;			/* erase HS3-HS0 */
		cfs->sel_card_head |= (((lba_addr >> 24) & 0x0f) | CF_LBA);
		/* write multiples sectors */
		if (n_sectors >= 256) {
			m_sectors = 256;
			cfs->sec_cnt = 0;
		} else {
			m_sectors = n_sectors;
			cfs->sec_cnt = n_sectors;
		}
		/* send command to write */
		if (!cyc_cf_send_cmd(cmd)) {
			return 0; 			/* command failed */
		}
		/* next loop */
		lba_addr += m_sectors;
		n_sectors -= m_sectors;
		while (m_sectors--) {
			/* wait for DRQ */
			if (!cyc_cf_check_status(CF_NLOOP * CF_DELAY, CF_STATUS_DRQ, 1)) {
				/* timeout waiting CF get ready */
				return 0;
			}
			/* write 256 words */
			fifo_size = 256;
			while (fifo_size--) {
				cfs->data = *buffer++;			
			}
			/* wait for writing data on device */
			delay_us(CF_DELAY);
			if (!cyc_cf_check_status(CF_NLOOP * CF_DELAY, CF_STATUS_BUSY, 0)) {
				/* timeout waiting CF get ready */
				return(0);
			}
		}
		dummy_status = cfs->status_and_command;
	}
	return 1;
}
