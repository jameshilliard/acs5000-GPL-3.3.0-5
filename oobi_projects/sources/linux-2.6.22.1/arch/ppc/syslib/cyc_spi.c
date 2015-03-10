/*
 *  8/00  -  CPM SPI works, so just use that
 *
 *  Copyright (c) 2000  Montavista Software, Inc <source@mvista.com>
 *  based on the following
 *
 */

/*-----------------------------------------------------------------------------
 *
 * Copyright (C) Cyclades Corporation, 1997 - 2000
 * All rights reserved.
 *
 * TSx100/NL1000 v1.00 for Linux
 *
 * This file :      cyc_spi.c
 *
 * Description:     This file contains the routines to access for SPI protocol
 *
 * Complies with Cyclades SW coding Standard rev 1.2
 *-----------------------------------------------------------------------------
 */

 /*----------------------------------------------------------------------------
  * Change History
  *----------------------------------------------------------------------------
  * 11/8/01   Arnaldo Zimmermann     Initial implementation
  *----------------------------------------------------------------------------
  * Aug/09/02 Edson Seabra           Port to linux kernel
  *----------------------------------------------------------------------------
  */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/timex.h>
#include <linux/time.h>

#include <asm/uaccess.h>
#include <asm/mpc8xx.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/prxk.h>

#include <asm/delay.h>
#include <asm/commproc.h>
#include <asm/time.h>
#include <asm/todc.h>

#include <asm/cyc_osd.h>

#undef DEBUG_READ_SPI
#undef DEBUG_WRITE_SPI

#define IS_BOARD_KVMANA	(CYC_BOARD_TYPE == BOARD_KVM16 || \
		CYC_BOARD_TYPE == BOARD_KVM32)
#define IS_BOARD_KVMNET	(CYC_BOARD_TYPE == BOARD_KVMNET16 || \
		CYC_BOARD_TYPE == BOARD_KVMNET32)
#define IS_BOARD_KVMNETP	(CYC_BOARD_TYPE == BOARD_KVMP16 || \
		CYC_BOARD_TYPE == BOARD_KVMP32)
#define IS_BOARD_ONS	(CYC_BOARD_TYPE == BOARD_ONS441 || \
		CYC_BOARD_TYPE == BOARD_ONS481 || \
		CYC_BOARD_TYPE == BOARD_ONS841 || \
		CYC_BOARD_TYPE == BOARD_ONS881 || \
		CYC_BOARD_TYPE == BOARD_ONS442 || \
		CYC_BOARD_TYPE == BOARD_ONS482 || \
		CYC_BOARD_TYPE == BOARD_ONS842 || \
		CYC_BOARD_TYPE == BOARD_ONS882)
#define IS_BOARD_KVM	(IS_BOARD_KVMANA || IS_BOARD_KVMNET || \
		IS_BOARD_ONS || IS_BOARD_KVMNETP)


/* for the KVM */
static int spiinuse = 0;
static int spiopened = 0;
static int spiinitialized = 0;

unsigned long tbdf_vaddr = 0;
unsigned long rbdf_vaddr = 0;

int cyc_cpm_spi_open(char);
int cyc_cpm_spi_close(unsigned char);

static void
msdelay(uint t, uint interv) 
{
	uint i;
	volatile sysconf8xx_t *psiu = &(((immap_t *)IMAP_ADDR)->im_siu_conf);

	/* delay t miliseconds */
	for (i=0; i<t; i++) {
		udelay(interv);
	 	psiu->sc_swsr = 0x556c;
		psiu->sc_swsr = 0xaa39;
	}
}

void __init
cyc_cpm_spi_init(void)
{
	uint	mem_addr, dp_addr, reloc;
	cbd_t		*rbdf;
	cbd_t		*tbdf;
	volatile cpm8xx_t	*cp;
	volatile spi_t		*spp;
	volatile immap_t	*immap;

	if (IS_BOARD_KVM && spiinitialized) {
		return;
	}

	cp = cpmp;	/* Get pointer to Communication Processor */
	immap = (immap_t *)IMAP_ADDR;	/* and to internal registers */

	spp = (spi_t *)&cp->cp_dparam[PROFF_SPI];

	/* Check for and use a microcode relocation patch.
	*/
	if ((reloc = spp->spi_rpbase))
		spp = (spi_t *)&cp->cp_dpmem[spp->spi_rpbase];
		

	/* Initialize Port B SPI pins.
	*/
	cp->cp_pbpar &= ~0x00020000;
	cp->cp_pbpar |=  0x0000000E;
	cp->cp_pbdir |=  0x0002000F;
	cp->cp_pbodr &= ~0x0000000E;
	cp->cp_pbdat |=  0x00000001;
	
	if (IS_BOARD_KVM) {
        	immap->im_ioport.iop_papar &= ~0x00c0;
        	immap->im_ioport.iop_padir |= 0x00c0;
        	immap->im_ioport.iop_padat &= ~0x00c0;
	}

	/* Initialize the parameter ram.
	 * We need to make sure many things are initialized to zero,
	 * especially in the case of a microcode patch.
	 */
	spp->spi_rstate = 0;
	spp->spi_rdp = 0;
	spp->spi_rbptr = 0;
	spp->spi_rbc = 0;
	spp->spi_rxtmp = 0;
	spp->spi_tstate = 0;
	spp->spi_tdp = 0;
	spp->spi_tbptr = 0;
	spp->spi_tbc = 0;
	spp->spi_txtmp = 0;


	/* Allocate space for one transmit and one receive buffer
	 * descriptor in the DP ram.
	 */
	dp_addr = m8xx_cpm_dpalloc(sizeof(cbd_t) * 2);
	if (dp_addr == CPM_DP_NOSPACE) {
		panic("Warning! Failed to allocate tx/rx buffer on DP ram for SPI");
	}
	/* Set up the SPI parameters in the parameter ram.
	*/
	spp->spi_tbase = dp_addr;
	spp->spi_rbase = dp_addr + sizeof(cbd_t);

	/*
	 *  manually init if using microcode patch
	 */
	if(reloc)
	{
		spp->spi_rbptr = spp->spi_rbase;
		spp->spi_tbptr = spp->spi_tbase;
	}

	spp->spi_tfcr = 0x18;
	spp->spi_rfcr = 0x18;

	/* Set maximum receive size.
	*/
	spp->spi_mrblr = 32;

	/* Initialize Tx/Rx parameters.
	*/
	if (reloc == 0) {
		cp->cp_cpcr =
			mk_cr_cmd(CPM_CR_CH_SPI, CPM_CR_INIT_TRX) | CPM_CR_FLG;
		while (cp->cp_cpcr & CPM_CR_FLG);
	}
	/* The notes on the SPI relocation patch indicate the the init rx and 
	   tx command doesn't work.  It isn't clear if this also includes 
	   the individual rx and tx init commands, but at any rate it seems 
	   to work fine if we just skip these commands.
	*/
	else {
		cp->cp_cpcr =
			mk_cr_cmd(CPM_CR_CH_SPI, CPM_CR_INIT_TX) | CPM_CR_FLG;
		while (cp->cp_cpcr & CPM_CR_FLG);

		cp->cp_cpcr =
			mk_cr_cmd(CPM_CR_CH_SPI, CPM_CR_INIT_RX) | CPM_CR_FLG;
		while (cp->cp_cpcr & CPM_CR_FLG);
	}

	/* Set the buffer address.
	*/
	dp_addr = m8xx_cpm_dpalloc(32);
	mem_addr = (uint)(&cpmp->cp_dpmem[dp_addr]);
	if (dp_addr == CPM_DP_NOSPACE) {
		panic("Warning! Failed to allocate rx buffer on CPM ram for SPI");
	}
	memset((void *) mem_addr, 0, 32);

	rbdf = (cbd_t *)&cp->cp_dpmem[spp->spi_rbase];
	rbdf->cbd_bufaddr = iopa(mem_addr); 
	rbdf_vaddr = mem_addr;

	dp_addr = m8xx_cpm_dpalloc(32);
	mem_addr = (uint)(&cpmp->cp_dpmem[dp_addr]);
	if (dp_addr == CPM_DP_NOSPACE) {
		panic("Warning! Failed to allocate tx buffer on CPM ram for SPI");
	}
	memset((void *) mem_addr, 0, 32);

	tbdf = (cbd_t *)&cp->cp_dpmem[spp->spi_tbase];
	tbdf->cbd_bufaddr = iopa(mem_addr); 
	tbdf_vaddr = mem_addr;

	/* Clear event
	*/
	cp->cp_spie = ~0;

	/* No events 
	*/
	cp->cp_spim = 0;


	/*
     *  62.5KHz clock for the A/D
     *  8 bit 
     *  The datasheet specs for the ADS7843 indicate that it should work 
     *  with a clock rate of up to 2MHz, but we don't seem to be able to 
     *  get reliable touchpanel samples with a clock much faster than 
     *  62.5KHz.  This translates to a sample time of 768 microseconds 
     *  to sample both the X and Y coordinates.
     */

	if (IS_BOARD_KVM) {
		spiinuse = 0;
		spiinitialized = 1;
		spiopened = 0;
	} else {
		cp->cp_spmode = SPMODE_CI | SPMODE_CP | SPMODE_REV | 
			SPMODE_MSTR | SPMODE_DIV16 | ((8-1) << 4) | 0xF | 
			SPMODE_EN;
	}
	printk("SPI init done.\r\n");
	//last_reboot();

}


/* Read from SPI.
 * This is a two step process.  First, we send the "dummy" write
 * to set the device offset for the read.  Second, we perform
 * the read operation.
 */

ssize_t
cyc_cpm_spi_read(char *buf, size_t count)
{
	volatile spi_t *spp;
	volatile cbd_t	*tbdf, *rbdf;
	volatile cpm8xx_t	*cp;
	unsigned long flags;
	uint	reloc;

	cp = cpmp;	/* Get pointer to Communication Processor */
	spp = (spi_t *)&cp->cp_dparam[PROFF_SPI];

	if (IS_BOARD_KVM) {
		cyc_cpm_spi_open(SPI_FROM_RTC);
	}

	/* Check for and use a microcode relocation patch.
	*/
	if ((reloc = spp->spi_rpbase)) {
		spp = (spi_t *)&cp->cp_dpmem[spp->spi_rpbase];
	}

	if (count >= spp->spi_mrblr) {
		return 0;
	}

	tbdf = (cbd_t *)&cp->cp_dpmem[spp->spi_tbase];
	rbdf = (cbd_t *)&cp->cp_dpmem[spp->spi_rbase];

	memset((void *) tbdf_vaddr, 0, count);
	*(char*)(tbdf_vaddr) &= ~0x80;	/* read command */
	memset((void *) rbdf_vaddr,0,count);

	tbdf->cbd_datlen = count;
	tbdf->cbd_sc = BD_SC_READY|BD_SC_WRAP|BD_SC_INTRPT|BD_SC_LAST; 

	rbdf->cbd_datlen = 0;
	rbdf->cbd_sc = BD_SC_EMPTY|BD_SC_WRAP|BD_SC_INTRPT;


	/* Chip bug, set enable here. */

	cp->cp_pbdat &=  ~0x00000001;
	msdelay(2, 1000);

	local_irq_save(flags);

	cp->cp_spie = ~0;
	cp->cp_spcom |= 0x80;	/* Start Transmit */

	/* Wait for SPI transfer. */

	while(!(cp->cp_spie & 0x35)) {
		msdelay(1,10);
	}

	local_irq_restore(flags);

	cp->cp_pbdat |=   0x00000001;

	if (cp->cp_spie != 0x03) {
		printk("\r\nSPI error on reading RTC. Status = 0x%04x", 
			cp->cp_spie);
		return 0;
	}

	if (signal_pending(current))
		return -ERESTARTSYS;

	if (rbdf->cbd_datlen != count) {
		printk("cyc_cpm_spi_read %d of %d\n", rbdf->cbd_datlen, count);
	}

	if (rbdf->cbd_sc & BD_SC_EMPTY) {
		printk("SPI read complete but rbuf empty\n");
	}
	
	memcpy(buf, (void *) rbdf_vaddr, count);

#ifdef DEBUG_READ_SPI
{	size_t i;
	printk("RTC read: ");
	for (i = 0; i<count; i++) printk("%02x", (unsigned char)buf[i]);
	printk("\r\n");
}
#endif

	if (IS_BOARD_KVM) {
		cyc_cpm_spi_close(SPI_FROM_RTC);
	}
	return rbdf->cbd_datlen;
}


ssize_t
cyc_cpm_spi_write(char *buf, size_t count)
{
	volatile spi_t *spp;
	volatile cbd_t	*tbdf, *rbdf;
	volatile cpm8xx_t	*cp;
	unsigned long flags;
	uint	reloc, status;

	cp = cpmp;	/* Get pointer to Communication Processor */
	spp = (spi_t *)&cp->cp_dparam[PROFF_SPI];

	if (IS_BOARD_KVM) {
		cyc_cpm_spi_open(SPI_FROM_RTC);
	}

	/* Check for and use a microcode relocation patch.
	*/
	if ((reloc = spp->spi_rpbase) != 0) {
		spp = (spi_t *)&cp->cp_dpmem[spp->spi_rpbase];
	}

	if (count >= spp->spi_mrblr) {
		return 0;
	}

	tbdf = (cbd_t *)&cp->cp_dpmem[spp->spi_tbase];
	rbdf = (cbd_t *)&cp->cp_dpmem[spp->spi_rbase];

#ifdef DEBUG_WRITE_SPI
{	size_t i;
	printk("RTC write: ");
	for (i = 0; i<count; i++) printk("%02x", (unsigned char)buf[i]);
	printk("\r\n");
}
#endif

	//[RK]Nov/04/05 - copy the date
	memcpy((void *) tbdf_vaddr, buf, count);
	*(char*)(tbdf_vaddr) |= 0x80;	/* write command */
	memset((void *) rbdf_vaddr,0,count);

	tbdf->cbd_datlen = count;		/* Length */
	rbdf->cbd_datlen = 0;			/* Length */

	tbdf->cbd_sc = BD_SC_READY|BD_SC_WRAP|BD_SC_INTRPT|BD_SC_LAST; 
	rbdf->cbd_sc = BD_SC_EMPTY|BD_SC_WRAP|BD_SC_INTRPT;

	cp->cp_pbdat &=  ~0x00000001;
	msdelay(2, 1000);

	local_irq_save(flags);

	cp->cp_spie = ~0;
	cp->cp_spcom = 0x80;

	/* Wait for SPI transfer.
	*/
	while(!(status = (cp->cp_spie & 0x32))) {
		msdelay(1,10);
	}

	local_irq_restore(flags);

	cp->cp_pbdat |=   0x00000001;

	if (signal_pending(current))
		return -ERESTARTSYS;

	if (status != 0x02) {				// check if TxB is set w/o other errors
		printk("\r\nSPI error when reading RTC");
		return 0;
	}

	if (tbdf->cbd_sc & BD_SC_READY)
		printk("SPI ra complete but tbuf ready\n");

	if (IS_BOARD_KVM) {
		cyc_cpm_spi_close(SPI_FROM_RTC);
	}

	return count;
}

int 
cyc_cpm_spi_open(char from)
{
#ifdef DEBUG_WRITE_SPI
	printk("SPI open from = %d, spiinuse = %d, spiopened = %d\n", 
		from, spiinuse, spiopened);
#endif
	if (spiinuse) {
		if (spiinuse == from + 1) {
			spiopened ++;
			return 1;
		} else {
			return 0;
		}
	}
	
	if (from >= 0 && from <= 2) {
		volatile cpm8xx_t *cp = cpmp;
		volatile immap_t *immap = (immap_t *)IMAP_ADDR;
		if (IS_BOARD_KVMNETP) {
			switch (from) {
			case 0:
        			immap->im_ioport.iop_padat &= ~0x00c0;
				break;
			case 1:
        			immap->im_ioport.iop_padat |= 0x0080;
        			immap->im_ioport.iop_padat &= ~0x0040;
				break;
			case 2:
        			immap->im_ioport.iop_padat |= 0x00c0;
				break;
			}
		} else {
        		immap->im_ioport.iop_padat &= ~0x00c0;
        		immap->im_ioport.iop_padat |= 0x0040 * from;
		}
		spiinuse = from + 1;
		spiopened = 1;
		if (from == 0) {
			cp->cp_spmode = SPMODE_CI | SPMODE_CP | SPMODE_REV | 
				SPMODE_MSTR | SPMODE_DIV16 | ((8-1) << 4) | 
				0xF | SPMODE_EN;
		} else {
			cp->cp_spmode = SPMODE_CI | SPMODE_CP |  
				SPMODE_MSTR | ((16-1) << 4) | 5 | SPMODE_EN;
		}
	}
	return 1;
}

int 
cyc_cpm_spi_close(unsigned char from)
{
	volatile cpm8xx_t *cp = cpmp;
	volatile immap_t *immap = (immap_t *)IMAP_ADDR;
#ifdef DEBUG_WRITE_SPI
	printk("SPI close from = %d, spiinuse = %d, spiopened = %d\n", 
		from, spiinuse, spiopened);
#endif
	if (!spiopened || (spiinuse && spiinuse != from + 1)) {
		return 0;
	}
	if (--spiopened == 0) {
		if (IS_BOARD_KVMNETP) {
       			immap->im_ioport.iop_padat &= ~0x00c0;
		} else {
       			immap->im_ioport.iop_padat |= 0x00c0;
		}
		spiinuse = 0;
		cp->cp_spmode = 0;
	}
	return 1;
}

#define INV(x)	(x & 0xff) * 0x100 + (x / 0x100)

ssize_t
cyc_cpm_spi_osd_write(char from, unsigned short *buffer, int size)
{
	volatile spi_t *spp;
	volatile cbd_t	*tbdf, *rbdf;
	volatile cpm8xx_t	*cp;
	uint	reloc, i;
	
	if (spiinuse != from + 1) {
		return 0;
	}

	cp = cpmp;	/* Get pointer to Communication Processor */
	spp = (spi_t *)&cp->cp_dparam[PROFF_SPI];

	/* Check for and use a microcode relocation patch.
	*/
	if ((reloc = spp->spi_rpbase)) {
		spp = (spi_t *)&cp->cp_dpmem[spp->spi_rpbase];
	}

	tbdf = (cbd_t *)&cp->cp_dpmem[spp->spi_tbase];
	rbdf = (cbd_t *)&cp->cp_dpmem[spp->spi_rbase];

	if (tbdf->cbd_sc & BD_SC_READY) {
		return 0;
	}
	cp->cp_pbdat &=  ~0x00000001;

	udelay(1);

	for (i = 0; i < size; i ++) {
#ifdef DEBUG_WRITE_SPI
		printk("SPI write: %04x\r\n", buffer[i]);
#endif

		*(unsigned short *)(tbdf_vaddr) = INV(buffer[i]);
		tbdf->cbd_datlen = 2;		/* Length */
		tbdf->cbd_sc = BD_SC_READY|BD_SC_WRAP|BD_SC_INTRPT|BD_SC_LAST; 
		rbdf->cbd_sc = BD_SC_EMPTY|BD_SC_WRAP|BD_SC_INTRPT;

		cp->cp_spie = ~0;
		cp->cp_spcom = 0x80;
		
		udelay(10);
		while (tbdf->cbd_sc & BD_SC_READY) {
			udelay(2);
		}
		
	}
	cp->cp_pbdat |=  0x00000001;
	udelay(2);
	return 2 * i;
}

