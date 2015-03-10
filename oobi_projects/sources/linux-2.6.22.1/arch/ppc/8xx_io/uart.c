/*
 * BK Id: SCCS/s.uart.c 1.22 11/10/01 15:35:06 trini
 */
/*
 *  UART driver for MPC860 CPM SCC or SMC
 *  Copyright (c) 1997 Dan Malek (dmalek@jlc.net)
 *
 * I used the serial.c driver as the framework for this driver.
 * Give credit to those guys.
 * The original code was written for the MBX860 board.  I tried to make
 * it generic, but there may be some assumptions in the structures that
 * have to be fixed later.
 * To save porting time, I did not bother to change any object names
 * that are not accessed outside of this file.
 * It still needs lots of work........When it was easy, I included code
 * to support the SCCs, but this has never been tested, nor is it complete.
 * Only the SCCs support modem control, so that is not complete either.
 *
 * This module exports the following rs232 io functions:
 *
 *	int rs_8xx_init(void);
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/tty_driver.h>
#include <linux/types.h>       /* needed? -- Tom */
#include <linux/serial.h>
#include <linux/serialP.h>
#include <linux/serial_reg.h>  /* needed? -- Tom */

#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/ptrace.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <asm/uaccess.h>
#include <asm/8xx_immap.h>
#include <asm/mpc8xx.h>
#include <asm/commproc.h>

#ifdef CONFIG_PRxK
#include <asm/prxk.h>
#define ASYNC_LEGACY_RTS        0x00200000 /* legacy RTS control */
#define ASYNC_LEGACY_CTS        0x00100000 /* legayc CTS control */

#define FRAMEMODE_PAGESIZE     4096 //as for 08-Ago-2002
#define FRAMEMODE_OTHERUSE     512  // the original value was 8, change due to I2C driver
#define FRAMEMODE_TOTALBUFF    8

//FRAMEMODE_FRAMELEN cannot be greater than 512
#define FRAMEMODE_FRAMELEN 	((FRAMEMODE_PAGESIZE - FRAMEMODE_OTHERUSE) / FRAMEMODE_TOTALBUFF)
//they last longer than an actual character !!!
//Modbus RTU framming requires 3,5 silent characters
#define FRAMEMODE_IDLEWORDS	  3 
//receive buffers are more critical...
#define FRAMEMODE_TXBUFFERS (FRAMEMODE_TOTALBUFF / 3)
#define FRAMEMODE_RXBUFFERS (FRAMEMODE_TOTALBUFF - FRAMEMODE_TXBUFFERS)

struct frame_mode_stats {
	int success;	// Number of frames correctly received
	int error;	// Number of frames received with errors
};

struct frame_mode_cfg {
	int status;	// 0 is off and 1 is on
	int mode;	// 0 for ascii framing and 1 for RTU framing
	uint txbuf;
	short txcount;
	short rxcount;
	short frame_started;   // tell if there is any frame currently being received
	struct frame_mode_stats rx_stat;	// Statistic for rx frames
	struct frame_mode_stats tx_stat;	// Statistic for tx frames
};

#endif

#ifdef CONFIG_MAGIC_SYSRQ
#include <linux/sysrq.h>
#endif

#ifdef CONFIG_KGDB
#include <asm/kgdb.h>
#endif

#ifdef CONFIG_PRxK
// az - acs1 only
#define MODEM_CONTROL
#define DO_THIS_LATER
static struct timeval  rtuend;  //used for Modbus RTU framming
struct timer_list wait_idle_line; //bug 3087
static volatile immap_t *immap = (immap_t *)IMAP_ADDR;

#define PORT_DTR1_PAR immap->im_cpm.cp_pbpar
#define PORT_DTR1_DAT immap->im_cpm.cp_pbdat
#define PORT_DTR1_DIR immap->im_cpm.cp_pbdir
#define CONFIG_UART_MAXIDL_SCC1 1
#define CONFIG_RTS1_PIN 19
#define CONFIG_DTR1_PIN_ACS1 20
#define CONFIG_DTR1_PIN_KVM 23
#define RTS1_PIN (1 << (31 - CONFIG_RTS1_PIN))
#define ITF_MODE_MASK   0x20    // PA10
#define ITF_WIRE_MASK   0x40    // PA9
#define ITF_OP_MASK             0x80    // PA8
#define ITF_MODE_RS232(port)    ((ITF_MODE_MASK & (port)) == 0x00)
#define ITF_MODE_RS485(port)    ((ITF_MODE_MASK & (port)) == 0x20)
#define ITF_WIRE_2(port)                ((ITF_WIRE_MASK & (port)) == 0x40)
#define ITF_WIRE_4(port)                ((ITF_WIRE_MASK & (port)) == 0x00)
#define ITF_OP_DISABLE(port)    ((ITF_OP_MASK & (port)) == 0x80)
#define ITF_OP_ENABLE(port)             ((ITF_OP_MASK & (port)) == 0x00)
unsigned int dtr1_pin;
#endif

#ifdef CONFIG_SERIAL_CONSOLE
#include <linux/console.h>

/* this defines the index into rs_table for the port to use
*/
# ifndef CONFIG_SERIAL_CONSOLE_PORT
#  ifdef CONFIG_SCC3_ENET
#   ifdef CONFIG_CONS_SMC2
#    define CONFIG_SERIAL_CONSOLE_PORT	0	/* Console on SMC2 is 1st port */
#   else
#    error "Can't use SMC1 for console with Ethernet on SCC3"
#   endif
#  else	/* ! CONFIG_SCC3_ENET */
#   ifdef CONFIG_CONS_SMC2			/* Console on SMC2 */
#    define CONFIG_SERIAL_CONSOLE_PORT	1
#   else					/* Console on SMC1 */
#    define CONFIG_SERIAL_CONSOLE_PORT	0
#   endif /* CONFIG_CONS_SMC2 */
#  endif  /* CONFIG_SCC3_ENET */
# endif	  /* CONFIG_SERIAL_CONSOLE_PORT */
#endif	  /* CONFIG_SERIAL_CONSOLE */

#if 0
/* SCC2 for console
*/
#undef CONFIG_SERIAL_CONSOLE_PORT
#define CONFIG_SERIAL_CONSOLE_PORT	2
#endif

#define TX_WAKEUP	ASYNC_SHARE_IRQ

#ifdef CONFIG_PRxK
#define IS_BOARD_KVMANA (CYC_BOARD_TYPE == BOARD_KVM16 || \
		CYC_BOARD_TYPE == BOARD_KVM32)
#define IS_BOARD_KVMNET (CYC_BOARD_TYPE == BOARD_KVMNET16 || \
		CYC_BOARD_TYPE == BOARD_KVMNET32)
#define IS_BOARD_KVMNETP        (CYC_BOARD_TYPE == BOARD_KVMP16 || \
		CYC_BOARD_TYPE == BOARD_KVMP32)
#define IS_BOARD_ONS    (CYC_BOARD_TYPE == BOARD_ONS441 || \
		CYC_BOARD_TYPE == BOARD_ONS481 || \
		CYC_BOARD_TYPE == BOARD_ONS841 || \
		CYC_BOARD_TYPE == BOARD_ONS881 || \
		CYC_BOARD_TYPE == BOARD_ONS442 || \
		CYC_BOARD_TYPE == BOARD_ONS482 || \
		CYC_BOARD_TYPE == BOARD_ONS842 || \
		CYC_BOARD_TYPE == BOARD_ONS882)
#define IS_BOARD_KVM    (IS_BOARD_KVMANA || IS_BOARD_KVMNET || \
		IS_BOARD_ONS)
#define IS_BOARD_ACS1           (CYC_BOARD_TYPE == BOARD_ACS1 || CYC_BOARD_TYPE == BOARD_AVCS1 || CYC_BOARD_TYPE == BOARD_ACS5001)
#define IS_BOARD_TS1H           (CYC_BOARD_TYPE == BOARD_TS1H)
#define IS_BOARD_ACS1_KVM       (IS_BOARD_ACS1 || IS_BOARD_KVM)
#define IS_BOARD_ACS1_KVM_TS1H  (IS_BOARD_ACS1 || IS_BOARD_KVM || IS_BOARD_TS1H)

#endif

static char *serial_name = "CPM UART driver";
static char *serial_version = "0.03";

static struct tty_driver *serial_driver;

#if defined(CONFIG_SERIAL_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
static unsigned long break_pressed; /* break, really ... */
#endif

/*
 * Serial driver configuration section.  Here are the various options:
 */
#define SERIAL_PARANOIA_CHECK
#define CONFIG_SERIAL_NOPAUSE_IO
#define SERIAL_DO_RESTART

/* Set of debugging defines */

#undef SERIAL_DEBUG_INTR
#undef SERIAL_DEBUG_OPEN
#undef SERIAL_DEBUG_FLOW
#undef SERIAL_DEBUG_SCC
#undef SERIAL_DEBUG_RS_WAIT_UNTIL_SENT

#define _INLINE_ inline
  
#define DBG_CNT(s)

/* We overload some of the items in the data structure to meet our
 * needs.  For example, the port address is the CPM parameter ram
 * offset for the SCC or SMC.  The maximum number of ports is 4 SCCs and
 * 2 SMCs.  The "hub6" field is used to indicate the channel number, with
 * a flag indicating SCC or SMC, and the number is used as an index into
 * the CPM parameter area for this device.
 * The "type" field is currently set to 0, for PORT_UNKNOWN.  It is
 * not currently used.  I should probably use it to indicate the port
 * type of SMC or SCC.
 * The SMCs do not support any modem control signals.
 */
#define smc_scc_num	hub6
#define NUM_IS_SCC	((int)0x00010000)
#define PORT_NUM(P)	((P) & 0x0000ffff)
#define PORT_IS_SCC(P)	((P) & NUM_IS_SCC)

/* The choice of serial port to use for KGDB.  If the system has
 * two ports, you can use one for console and one for KGDB (which
 * doesn't make sense to me, but people asked for it).
 */
#if defined(CONFIG_KGDB_TTYS3)
#define KGDB_SER_IDX 3
#elif defined(CONFIG_KGDB_TTYS2)
#define KGDB_SER_IDX 2
#elif defined(CONFIG_KGDB_TTYS1)
#define KGDB_SER_IDX 1
#else
#define KGDB_SER_IDX 0
#endif

/* Processors other than the 860 only get SMCs configured by default.
 * Either they don't have SCCs or they are allocated somewhere else.
 * Of course, there are now 860s without some SCCs, so we will need to
 * address that someday.
 * The Embedded Planet Multimedia I/O cards use TDM interfaces to the
 * stereo codec parts, and we use SMC2 to help support that.
 */
static struct serial_state rs_table[] = {
	/* UART CLK   PORT          IRQ      FLAGS  NUM   */
#ifndef CONFIG_PRxK
#ifndef CONFIG_SCC3_ENET	/* SMC1 not usable with Ethernet on SCC3 */
  	{ 0,     0, PROFF_SMC1, CPMVEC_SMC1,   0,    0 },    /* SMC1 ttyS0 */
#endif
#if !defined(CONFIG_USB_MPC8xx) && !defined(CONFIG_USB_CLIENT_MPC8xx)
# ifdef CONFIG_SMC2_UART
  	{ 0,     0, PROFF_SMC2, CPMVEC_SMC2,   0,    1 },    /* SMC2 ttyS1 */
# endif
# ifdef CONFIG_USE_SCC_IO
  	{ 0,     0, PROFF_SCC2, CPMVEC_SCC2,   0,    (NUM_IS_SCC | 1) },    /* SCC2 ttyS2 */
  	{ 0,     0, PROFF_SCC3, CPMVEC_SCC3,   0,    (NUM_IS_SCC | 2) },    /* SCC3 ttyS3 */
# endif
  #else /* CONFIG_USB_xxx */
# ifdef CONFIG_USE_SCC_IO
  	{ 0,     0, PROFF_SCC3, CPMVEC_SCC3,   0,    (NUM_IS_SCC | 2) },    /* SCC3 ttyS3 */
# endif
#endif	/* CONFIG_USB_xxx */
#else	/* CONFIG_PRxK */
#ifdef CONFIG_TS1H
	{ 0,     0, PROFF_SCC1, CPMVEC_SCC1,   0,    (NUM_IS_SCC | 0) },    /* SCC1 ttyS0 - TS1H */
#else
  	{ 0,     0, PROFF_SMC1, CPMVEC_SMC1,   0,    0 },    /* SMC1 ttyS0 */
    { 0,     0, PROFF_SCC1, CPMVEC_SCC1,   0,    (NUM_IS_SCC | 0) },    /* SCC1 ttyS1 */
#endif
#endif	/* CONFIG_PRxK */
};

#define NR_PORTS	(sizeof(rs_table)/sizeof(struct serial_state))


/* The number of buffer descriptors and their sizes.
*/
#define RX_NUM_FIFO	4
#define RX_BUF_SIZE	32
#define TX_NUM_FIFO	4
#define TX_BUF_SIZE	32

#ifdef CONFIG_PRxK
#define RX_SCC_NUM_FIFO FRAMEMODE_RXBUFFERS
#define RX_SCC_BUF_SIZE FRAMEMODE_FRAMELEN
#define TX_SCC_NUM_FIFO FRAMEMODE_TXBUFFERS
#define TX_SCC_BUF_SIZE FRAMEMODE_FRAMELEN
#endif

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

/* The async_struct in serial.h does not really give us what we
 * need, so define our own here.
 */
typedef struct serial_info {
	int			magic;
	int			flags;
	struct serial_state	*state;
	struct tty_struct 	*tty;
	int			read_status_mask;
	int			ignore_status_mask;
	int			timeout;
	int			line;
	int			x_char;	/* xon/xoff character */
	int			close_delay;
	unsigned short		closing_wait;
	unsigned short		closing_wait2;
	unsigned long		event;
	unsigned long		last_active;
	int			blocked_open; /* # of blocked opens */
	long			session; /* Session of opening process */
	long			pgrp; /* pgrp of opening process */
	struct work_struct      work;
	struct work_struct      hangup;
	wait_queue_head_t	open_wait;
	wait_queue_head_t	close_wait;

	/* CPM Buffer Descriptor pointers.
	*/
	cbd_t			*rx_bd_base;
	cbd_t			*rx_cur;
	cbd_t			*tx_bd_base;
	cbd_t			*tx_cur;

       /* Virtual addresses for the FIFOs because we can't __va() a
        * physical address anymore.
        */
        unsigned char          *rx_va_base;
        unsigned char          *tx_va_base;
#ifdef CONFIG_PRxK
	spinlock_t  lock;
	unsigned int            MCR;
	unsigned int            MSR;
	unsigned int            IER;
	wait_queue_head_t       delta_msr_wait;
	struct frame_mode_cfg frame_cfg;
	unsigned char   hw_stopped;     // Half duplex CTS ctrl
	unsigned char   media;          // RS485
	unsigned char   terminator;     // RS485 with terminator
	cbd_t           *bdtx;
#endif

} ser_info_t;

static inline int waitBdpReady(ser_info_t *info, volatile cbd_t  *bdp);
static inline int isDeviceConnected(ser_info_t *info);
static int __init serial_console_setup(struct console *co, char *options);
static struct tty_driver *serial_console_device(struct console *c, int *index);
static void serial_console_write(struct console *c, const char *s,unsigned count);

static struct console sercons = {
	name:		"ttyS",
	write:		serial_console_write,
	device:		serial_console_device,
	setup:		serial_console_setup,
	flags:		CON_PRINTBUFFER,
	index:		CONFIG_SERIAL_CONSOLE_PORT,
};

#ifdef CONFIG_PRxK
static void do_serial_hangup(struct work_struct *private_);
#endif

static void change_speed(ser_info_t *info);
static void rs_8xx_wait_until_sent(struct tty_struct *tty, int timeout);
static int rs_8xx_write(struct tty_struct * tty,
			const unsigned char *buf, int count);

/* Connect a baud rate generator to an SMC or SCC.
 * brg is the zero-based baud rate generator index, e.g. brg == 0 selects BRG1.
 * Legal values for brg are 0, 1, 2, and 3.
 */
static void connect_brg(const struct serial_state *state, int brg)
{
	int idx;

	if (brg < 0 || brg > 3)
		return;

	idx = PORT_NUM(state->smc_scc_num);
	if (PORT_IS_SCC(state->smc_scc_num)) {
		unsigned int sicr_mask = (0xff << (idx * 8));
		unsigned int sicr_val  = (((brg << 3 ) | brg) << (idx * 8));

		cpmp->cp_sicr &= ~sicr_mask;
		cpmp->cp_sicr |=  sicr_val;
	}
	else {
		unsigned int simode_mask = (0xf000 << (idx * 16));
		unsigned int simode_val  = ((brg << 12) << (idx * 16));

		cpmp->cp_simode &= ~simode_mask;
		cpmp->cp_simode |=  simode_val;
	}
	return;
}

#ifdef MODEM_CONTROL
static void serial_out(ser_info_t *info, int val, int val2)
{
	int pin_value;
	volatile scc_t *scp;
	immap_t *immap = (immap_t *)IMAP_ADDR;

	// is console ==> return
	if (!IS_BOARD_TS1H) {
		if (!IS_BOARD_ACS1_KVM ||
		    !(PORT_IS_SCC(info->state->smc_scc_num)))
			return;
        }

	if (val == UART_MCR) { // modem control register
		local_irq_disable();
		pin_value = immap->im_cpm.cp_pbdat;
		if (val2 & UART_MCR_DTR) {
			pin_value &= ~dtr1_pin;
		} else {
			pin_value |= dtr1_pin;
		}
		if (!info->media) { // RS232
			if (val2 & UART_MCR_RTS) {
				pin_value &= ~RTS1_PIN;
			} else {
				pin_value |= RTS1_PIN;
			}
		}

		immap->im_cpm.cp_pbdat = pin_value;
		local_irq_enable();

		if (info->media && (info->flags & ASYNC_LEGACY_RTS)) { // RS485-Half
			if (val2 & UART_MCR_RTS) { // RTS ON
				if (info->terminator)
					immap->im_ioport.iop_pcdat |= (1 << 9); // termination OFF
				immap->im_ioport.iop_padat &= ~ITF_OP_MASK; /*D_SEL*/
				scp = &immap->im_cpm.cp_scc[PORT_NUM(info->state->smc_scc_num)];
				scp->scc_sccm &= ~(UART_SCCM_RX | UART_SCCM_BSY);
				scp->scc_gsmrl &= ~SCC_GSMRL_ENR;
			} else { // RTS OFF
				immap->im_ioport.iop_padat |= ITF_OP_MASK;
				if (info->terminator)
					immap->im_ioport.iop_pcdat &= ~(1 << 9); // termination ON
				scp = &immap->im_cpm.cp_scc[PORT_NUM(info->state->smc_scc_num)];
				scp->scc_sccm |= (UART_SCCM_RX | UART_SCCM_BSY);
				scp->scc_gsmrl |= SCC_GSMRL_ENR;
			}
		}
	} else {
		if (val == UART_IER) {
			// Modem interrupt and RS232
			pin_value = 0;
			immap->im_ioport.iop_pcso &= 0xffef; // CTS1 as GP I/O
			if (val2 & UART_IER_MSI) {
				// CTS1 connected to the pin and can generate interrupt
				pin_value = 0x10;
			} else {
				// CTS1 always asserted and can generate interrupt
				pin_value = 0x00;
			}
			immap->im_ioport.iop_pcso |= pin_value;
		}
	}
}

int serial_in(ser_info_t *info, int val)
{
	int status = 0;
	immap_t *immap = (immap_t *)IMAP_ADDR;

	if (val == UART_MSR) {
		status =  ((immap->im_ioport.iop_pcdat & 0x20) ? 0 : UART_MSR_DCD)
			   | ((immap->im_ioport.iop_pcdat & 0x8) ? 0 : UART_MSR_DSR)
			   | ((immap->im_ioport.iop_pcdat & 0x10) ? 0 : UART_MSR_CTS)
				;
	
		if ((status & UART_MSR_DCD) != (info->MSR & UART_MSR_DCD)){
			status |= UART_MSR_DDCD;
		}

		if ((status & UART_MSR_DSR) != (info->MSR & UART_MSR_DSR)) {
			status |= UART_MSR_DDSR;
		}

		if ((status & UART_MSR_CTS) != (info->MSR & UART_MSR_CTS)) {
			status |= UART_MSR_DCTS;
		}

		info->MSR = status;
	}

	if (val == UART_MCR) {
		status = ((immap->im_cpm.cp_pbdat & 0x800) ? 0 : UART_MCR_DTR)
			  | ((immap->im_cpm.cp_pbdat & 0x1000) ? 0 : UART_MCR_RTS)
				;
	}

	if (val == UART_LSR) {
		status = 0;
	}

	return(status);
}

#endif

static inline int serial_paranoia_check(ser_info_t *info,
					char* device, const char *routine)
{
#ifdef SERIAL_PARANOIA_CHECK
	static const char *badmagic =
		"Warning: bad magic number for serial struct (%s) in %s\n";
	static const char *badinfo =
		"Warning: null async_struct for (%s) in %s\n";

	if (!info) {
		printk(badinfo, device, routine);
		return 1;
	}
	if (info->magic != SERIAL_MAGIC) {
		printk(badmagic, device, routine);
		return 1;
	}
#endif
	return 0;
}

/*
 * This is used to figure out the divisor speeds and the timeouts,
 * indexed by the termio value.  The generic CPM functions are responsible
 * for setting and assigning baud rate generators for us.
 */
static int baud_table[] = {
	0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800,
	9600, 14400, 19200, 28800, 38400, 57600, 76800, 115200, 230400, 
	460800, 0 };


/*
 * ------------------------------------------------------------
 * rs_stop() and rs_start()
 *
 * This routines are called before setting or resetting tty->stopped.
 * They enable or disable transmitter interrupts, as necessary.
 * ------------------------------------------------------------
 */
static void rs_8xx_stop(struct tty_struct *tty)
{
	ser_info_t *info = (ser_info_t *)tty->driver_data;
	int	idx;
	unsigned long flags;
	volatile scc_t	*sccp;
	volatile smc_t	*smcp;

	if (serial_paranoia_check(info, tty->name, "rs_stop"))
		return;
	
	local_irq_save(flags);
	idx = PORT_NUM(info->state->smc_scc_num);
	if (PORT_IS_SCC(info->state->smc_scc_num)) {
		sccp = &cpmp->cp_scc[idx];
		sccp->scc_sccm &= ~UART_SCCM_TX;
	}
	else {
		smcp = &cpmp->cp_smc[idx];
		smcp->smc_smcm &= ~SMCM_TX;
	}
	local_irq_restore(flags);
}

static void rs_8xx_start(struct tty_struct *tty)
{
	ser_info_t *info = (ser_info_t *)tty->driver_data;
	int	idx;
	unsigned long flags;
	volatile scc_t	*sccp;
	volatile smc_t	*smcp;

	if (serial_paranoia_check(info, tty->name, "rs_start"))
		return;
	
	idx = PORT_NUM(info->state->smc_scc_num);
	local_irq_save(flags);
	if (PORT_IS_SCC(info->state->smc_scc_num)) {
		sccp = &cpmp->cp_scc[idx];
		sccp->scc_sccm |= UART_SCCM_TX;
	}
	else {
		smcp = &cpmp->cp_smc[idx];
		smcp->smc_smcm |= SMCM_TX;
	}
	local_irq_restore(flags);
}

/*
 * ----------------------------------------------------------------------
 *
 * Here starts the interrupt handling routines.  All of the following
 * subroutines are declared as inline and are folded into
 * rs_interrupt().  They were separated out for readability's sake.
 *
 * Note: rs_interrupt() is a "fast" interrupt, which means that it
 * runs with interrupts turned off.  People who may want to modify
 * rs_interrupt() should try to keep the interrupt handler as fast as
 * possible.  After you are done making modifications, it is not a bad
 * idea to do:
 * 
 * gcc -S -DKERNEL -Wall -Wstrict-prototypes -O6 -fomit-frame-pointer serial.c
 *
 * and look at the resulting assemble code in serial.s.
 *
 * 				- Ted Ts'o (tytso@mit.edu), 7-Mar-93
 * -----------------------------------------------------------------------
 */

#ifdef CONFIG_PRxK
static _INLINE_ void receive_frame(ser_info_t *info)
{
	struct tty_struct *tty = info->tty;
	unsigned char ch, flag, *cp, *dest=0;
	int i, control_char = 0, len;
	ushort  status;
	struct  async_icount *icount;
	volatile cbd_t  *bdp;
	volatile scc_t  *sccp;
	struct tty_buffer *tb;

	icount = &info->state->icount;

	if (info->state->smc_scc_num & NUM_IS_SCC) {
		sccp = &cpmp->cp_scc[PORT_NUM(info->state->smc_scc_num)];
	}

	/* Just loop through the closed BDs and copy the characters into
	 * the buffer.
	 */
	bdp = info->rx_cur;
	for (;;) {
		volatile sysconf8xx_t *psiu = &(((immap_t *)IMAP_ADDR)->im_siu_conf);
		psiu->sc_swsr = 0x556c;
		psiu->sc_swsr = 0xaa39;

		if (bdp->cbd_sc & BD_SC_EMPTY) {    /* If this one is empty */
			break;                          /* we are all done */
		}

		/* Get the number of characters and the buffer pointer. */
		i = bdp->cbd_datlen;

		cp = (unsigned char *)__va(bdp->cbd_bufaddr);
		status = bdp->cbd_sc;

		/* Check to see if there is room in the tty buffer for
		 * the characters in our BD buffer.  If not, we exit
		 * now, leaving the BD with the characters.  We'll pick
		 * them up again on the next receive interrupt (which could
		 * be a timeout).
		 */

		if (i >= tty->receive_room)  //TODO: check this
			break;

		if (*cp == STOP_CHAR(tty) || *cp == START_CHAR(tty)) {
			control_char = 1;
			goto process_char;
		}

		// if we receive a ':' char it is a start-of-frame
		if (!info->frame_cfg.frame_started) {
			do {
				if ( *cp++ == 0x3a ) {
					len = tty_prepare_flip_string(tty, &dest, 2);
					if (len < 2) {
						printk("uart.c:receive_frame() - no space in flip\n");
						return;
					}
					info->frame_cfg.rxcount += 2;
					info->frame_cfg.frame_started = 1;
				} else {
					printk("uart.c:receive_frame() - wrong start character\n");
				}
			} while (--i && !info->frame_cfg.frame_started);
		}

		if (!i) {
			bdp->cbd_sc |= BD_SC_EMPTY;
			bdp->cbd_sc &= ~(BD_SC_BR | BD_SC_FR | BD_SC_PR | BD_SC_OV);
			if (bdp->cbd_sc & BD_SC_WRAP)
				bdp = info->rx_bd_base;
			else
				bdp++;
			info->rx_cur = (cbd_t *)bdp;
			return;
		}
process_char:

		while (i-- > 0) {
			ch = *cp++;
			icount->rx++;
			info->frame_cfg.rxcount++;
			flag = TTY_NORMAL;

			if (status & (BD_SC_BR | BD_SC_FR | BD_SC_PR | BD_SC_OV)) {
				/* For statistics only */
				if (status & BD_SC_BR)
					icount->brk++;
				else if (status & BD_SC_PR)
					icount->parity++;
				else if (status & BD_SC_FR)
					icount->frame++;
				if (status & BD_SC_OV)
					icount->overrun++;
				
				status &= info->read_status_mask;

				if (status & (BD_SC_BR)) {
					flag = TTY_BREAK;
					if (info->flags & ASYNC_SAK)
						do_SAK(tty);
				} else 
					if (status & BD_SC_PR)
						flag = TTY_PARITY;
					else 
						if (status & BD_SC_FR)
							flag = TTY_FRAME;
				tty_insert_flip_char(tty, ch, flag);
				if (status & BD_SC_OV) {
					/* Overrun is special, since it's
					 * reported immediately, and doesn't
					 * affect the current character
					 */
					tty_insert_flip_char(tty, 0, TTY_OVERRUN);
				}
			}
		}

		tb = tty->buf.tail;
		if ( !control_char &&
			 *((char *)(tb->char_buf_ptr + tb->used - 1)) == 0x0a &&
			 *((char *)(tb->char_buf_ptr + tb->used - 2)) == 0x0d ) {
			tb->used -=2;
			info->frame_cfg.rxcount = 0;
			info->frame_cfg.frame_started = 0;
			info->frame_cfg.rx_stat.success++;
		}

		/* This BD is ready to be used again.  Clear status.
		 * Get next BD.
		 */
		bdp->cbd_sc |= BD_SC_EMPTY;
		bdp->cbd_sc &= ~(BD_SC_BR | BD_SC_FR | BD_SC_PR | BD_SC_OV);

		if (bdp->cbd_sc & BD_SC_WRAP)
			bdp = info->rx_bd_base;
		else
			bdp++;
	}

	info->rx_cur = (cbd_t *)bdp;

	tty_schedule_flip(tty);
}

#endif

static _INLINE_ void receive_chars(ser_info_t *info)
{
	struct tty_struct *tty = info->tty;
	unsigned char ch, flag, *cp;
	int	i;
	ushort	status;
	struct	async_icount *icount;
	volatile cbd_t	*bdp;
	int rx_buf_size = RX_BUF_SIZE;

	icount = &info->state->icount;

	if (PORT_IS_SCC(info->state->smc_scc_num)) {
		rx_buf_size = RX_SCC_BUF_SIZE;
	}

	/* Just loop through the closed BDs and copy the characters into
	 * the buffer.
	 */
	bdp = info->rx_cur;
	for (;;) {
#ifdef CONFIG_PRxK
		volatile sysconf8xx_t *psiu = &(((immap_t *)IMAP_ADDR)->im_siu_conf);
		psiu->sc_swsr = 0x556c;
		psiu->sc_swsr = 0xaa39;
#endif
		if (bdp->cbd_sc & BD_SC_EMPTY)	/* If this one is empty */
			break;			/*   we are all done */

		/* Get the number of characters and the buffer pointer.
		*/
		i = bdp->cbd_datlen;
		cp = info->rx_va_base + ((bdp - info->rx_bd_base) * rx_buf_size);
		status = bdp->cbd_sc;
#ifdef CONFIG_KGDB
		if (info->line == KGDB_SER_IDX) {
			if (*cp == 0x03 || *cp == '$')
				breakpoint();
			return;
		}
#endif

		/* Check to see if there is room in the tty buffer for
		 * the characters in our BD buffer.  If not, we exit
		 * now, leaving the BD with the characters.  We'll pick
		 * them up again on the next receive interrupt (which could
		 * be a timeout).
		 */

#ifdef CONFIG_PRxK
		if (i >= tty->receive_room) //TODO: check this
			break;

		if ( info->frame_cfg.status == FRAMEMODE_ON ) { //surely it is RTU
			short aux = i + 2;

			tty_insert_flip_string(tty, (char *)&aux, sizeof(aux));
			info->frame_cfg.rx_stat.success++;
		}
#endif

		while (i-- > 0) {
			ch = *cp++;
			icount->rx++;

#ifdef SERIAL_DEBUG_INTR
			printk("DR%02x:%02x...", ch, status);
#endif
			flag = TTY_NORMAL;
			if (status & (BD_SC_BR | BD_SC_FR |
				       BD_SC_PR | BD_SC_OV)) {
				/*
				 * For statistics only
				 */
				if (status & BD_SC_BR)
					icount->brk++;
				else if (status & BD_SC_PR)
					icount->parity++;
				else if (status & BD_SC_FR)
					icount->frame++;
				if (status & BD_SC_OV)
					icount->overrun++;

				/*
				 * Now check to see if character should be
				 * ignored, and mask off conditions which
				 * should be ignored.
				if (status & info->ignore_status_mask) {
					if (++ignored > 100)
						break;
					continue;
				}
				 */
				status &= info->read_status_mask;

				if (status & (BD_SC_BR)) {
#ifdef SERIAL_DEBUG_INTR
					printk("handling break....");
#endif
					flag = TTY_BREAK;;
					if (info->flags & ASYNC_SAK)
						do_SAK(tty);
				} else if (status & BD_SC_PR)
					flag = TTY_PARITY;
				else if (status & BD_SC_FR)
					flag = TTY_FRAME;
			}
#if defined(CONFIG_SERIAL_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
			if (break_pressed && info->line == sercons.index) {
				if (ch != 0 && time_before(jiffies, 
							break_pressed + HZ*5)) {
					handle_sysrq(ch, tty);
					break_pressed = 0;
					goto ignore_char;
				} else
					break_pressed = 0;
			}
#endif
			tty_insert_flip_char(tty, ch, flag);
			if (status & BD_SC_OV) {
				/*
				 * Overrun is special, since it's
				 * reported immediately, and doesn't
				 * affect the current character
				 */
				tty_insert_flip_char(tty, 0, TTY_OVERRUN);
			}
		}

#if defined(CONFIG_SERIAL_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
	ignore_char:	
#endif
		/* This BD is ready to be used again.  Clear status.
		 * Get next BD.
		 */
		bdp->cbd_sc |= BD_SC_EMPTY;
		bdp->cbd_sc &= ~(BD_SC_BR | BD_SC_FR | BD_SC_PR | BD_SC_OV);

		if (bdp->cbd_sc & BD_SC_WRAP)
			bdp = info->rx_bd_base;
		else
			bdp++;
	}
	info->rx_cur = (cbd_t *)bdp;

	tty_schedule_flip(tty);
}

static _INLINE_ void receive_break(ser_info_t *info)
{
	struct tty_struct *tty = info->tty;

	info->state->icount.brk++;

#if defined(CONFIG_SERIAL_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
	if (info->line == sercons.index) {
		if (!break_pressed) {
			break_pressed = jiffies;
			return;
		} else
			break_pressed = 0;
	}
#endif

	/* Check to see if there is room in the tty buffer for
	 * the break.  If not, we exit now, losing the break.  FIXME
	 */

	tty_insert_flip_char(tty, 0, TTY_BREAK);
	tty_schedule_flip(tty);
}

static _INLINE_ void transmit_chars(ser_info_t *info)
{
#ifdef MODEM_CONTROL
	volatile cbd_t *bdpa;

	if (!info->tty) return;

	if (IS_BOARD_ACS1_KVM_TS1H &&
	    (PORT_IS_SCC(info->state->smc_scc_num))) {
		if ((info->flags & ASYNC_LEGACY_RTS) && info->bdtx) {
			bdpa = info->bdtx;
			if (!info->media && (bdpa->cbd_sc & BD_SC_CD)) { // CTS lost
				bdpa->cbd_sc |= BD_SC_READY | BD_SC_CR;
				bdpa->cbd_sc &= ~BD_SC_CD;
				return;
			}
			// verify next BD
			if (bdpa->cbd_sc & BD_SC_WRAP)
				bdpa = info->tx_bd_base;
			else
				bdpa++;

			if ((bdpa != info->tx_cur) && bdpa->cbd_datlen) {
				info->bdtx->cbd_datlen = 0;
				info->bdtx = (cbd_t *) bdpa;
				bdpa->cbd_sc |= BD_SC_READY | BD_SC_CR;
			} else {
				info->bdtx = 0;
				info->MCR &= ~UART_MCR_RTS; // RTS OFF
				serial_out(info, UART_MCR, info->MCR);
			}
		}
	}

#endif
	
	if ((info->flags & TX_WAKEUP) ||
	    (info->tty->flags & (1 << TTY_DO_WRITE_WAKEUP))) {
		info->event |= 1 << RS_EVENT_WRITE_WAKEUP;
		schedule_work(&info->work);
	}

#ifdef SERIAL_DEBUG_INTR
	printk("THRE...");
#endif
}

#ifdef MODEM_CONTROL
static _INLINE_ void check_modem_status(ser_info_t *info)
{
	int	status;
	struct	async_icount *icount;
	
	status = serial_in(info, UART_MSR);

	if (status & UART_MSR_ANY_DELTA) {
		icount = &info->state->icount;
		/* update input line counters */
		if (status & UART_MSR_TERI)
			icount->rng++;
		if (status & UART_MSR_DDSR)
			icount->dsr++;
		if (status & UART_MSR_DDCD) {
			icount->dcd++;
#ifdef CONFIG_HARD_PPS
			if ((info->flags & ASYNC_HARDPPS_CD) &&
			    (status & UART_MSR_DCD))
				hardpps();
#endif
		}
		if (status & UART_MSR_DCTS)
			icount->cts++;
		wake_up_interruptible(&info->delta_msr_wait);
	}

	if ((info->flags & ASYNC_CHECK_CD) && (status & UART_MSR_DDCD)) {
#if (defined(SERIAL_DEBUG_OPEN) || defined(SERIAL_DEBUG_INTR))
#ifdef SERIAL_DEBUG_SCC
		if (PORT_IS_SCC(info->state->smc_scc_num))
#endif
		printk("ttys%d CD now %s...", info->line,
		       (status & UART_MSR_DCD) ? "on" : "off");
#endif		
		if (status & UART_MSR_DCD) {
			printk(KERN_ALERT "PORT DCD: Port %d DCD went high",info->line);
			wake_up_interruptible(&info->open_wait);
		} else {
#ifdef SERIAL_DEBUG_OPEN
#ifdef SERIAL_DEBUG_SCC
			if (PORT_IS_SCC(info->state->smc_scc_num))
#endif
			printk("scheduling hangup...");
#endif
			printk(KERN_ALERT "PORT DCD: Port %d DCD went low", info->line);
			schedule_work(&info->hangup);
		}
	}

	if ((info->flags & ASYNC_LEGACY_CTS) &&
	    (status & UART_MSR_DCTS)) {
		if (status & UART_MSR_CTS) { // CTS ON
			info->hw_stopped = 1;
			if (!info->bdtx &&
			    (serial_in(info,UART_MCR) & UART_MCR_RTS)) {
				info->MCR &= ~UART_MCR_RTS; // RTS OFF
				serial_out(info,UART_MCR,info->MCR);
			}
		} else { // CTS OFF
			info->hw_stopped = 0;
			if ((info->flags & TX_WAKEUP) ||
			    (info->tty &&
			     (info->tty->flags & (1 << TTY_DO_WRITE_WAKEUP)))) {
				info->event |= 1 << RS_EVENT_WRITE_WAKEUP;
				schedule_work(&info->work);
			}
		}
	}
}

static void rs_8xx_dcd_interrupt(void *dev_id)
{
	ser_info_t * info;

	info = (ser_info_t *) dev_id;

	if (IS_BOARD_ACS1_KVM_TS1H &&
	    (PORT_IS_SCC(info->state->smc_scc_num)) &&
	    !info->media && (info->flags & ASYNC_CHECK_CD)) {
		check_modem_status(info);
	}
	info->last_active = jiffies;
}

static void rs_8xx_cts_interrupt(void *dev_id)
{
	ser_info_t * info;

	info = (ser_info_t *) dev_id;

	if (IS_BOARD_ACS1_KVM_TS1H &&
	    (PORT_IS_SCC(info->state->smc_scc_num)) &&
	    !info->media && (info->flags & ASYNC_LEGACY_CTS)) {
		check_modem_status(info);
	}
	info->last_active = jiffies;
}

#endif

/*
 * This is the serial driver's interrupt routine for a single port
 */
static void rs_8xx_interrupt(void *dev_id)
{
	u_char	events;
	int	idx;
	ser_info_t *info;
	volatile smc_t	*smcp;
	volatile scc_t	*sccp;
	
	info = (ser_info_t *)dev_id;

	idx = PORT_NUM(info->state->smc_scc_num);
	if (PORT_IS_SCC(info->state->smc_scc_num)) {
		sccp = &cpmp->cp_scc[idx];
		events = sccp->scc_scce;
		if (events & SMCM_BRKE)
			receive_break(info);
#ifdef CONFIG_PRxK
		sccp->scc_scce = events;
		if (events & (SCCM_RX | SCCM_BSY)) {
			if (IS_BOARD_ACS1_KVM_TS1H &&
			    (info->frame_cfg.status == FRAMEMODE_ON)  &&
			    (info->frame_cfg.mode == FRAMEMODE_ASCII)) {
				receive_frame(info);
			} else {
				receive_chars(info);
			}
		}
		if (events & SCCM_TX) {
			if (IS_BOARD_ACS1_KVM_TS1H &&
			    (info->frame_cfg.status == FRAMEMODE_ON) &&
			    (info->frame_cfg.mode == FRAMEMODE_RTU)) {
				long rtui;

				do_gettimeofday(&rtuend);

				// (FRAMEMODE_IDLEWORDS + 0,5) chars at 8N1 last "rtui" microseconds
				rtui = (FRAMEMODE_IDLEWORDS * 10 + 5) * 1042 * 960 /
				baud_table[info->tty->termios->c_cflag & CBAUD];

				if ((rtuend.tv_usec += rtui) > 999999) {
					rtuend.tv_usec -= 1000000;
					rtuend.tv_sec++;
				}
			}
			transmit_chars(info);
		}
#else
		if (events & SCCM_RX)
			receive_chars(info);
		if (events & SCCM_TX)
			transmit_chars(info);
		sccp->scc_scce = events;
#endif
	}
	else {
		smcp = &cpmp->cp_smc[idx];
		events = smcp->smc_smce;
#ifdef CONFIG_PRxK
		smcp->smc_smce = events;
		if (events & SMCM_BRKE)
			receive_break(info);
		if (events & (SMCM_RX | SMCM_BSY))
			receive_chars(info);
		if (events & SMCM_TX)
			transmit_chars(info);
#else
		if (events & SMCM_BRKE)
			receive_break(info);
		if (events & SMCM_RX)
			receive_chars(info);
		if (events & SMCM_TX)
			transmit_chars(info);
		smcp->smc_smce = events;
#endif
	}
	
#ifdef SERIAL_DEBUG_INTR
	printk("rs_interrupt_single(%d, %x)...",
					info->state->smc_scc_num, events);
#endif
	info->last_active = jiffies;
#ifdef SERIAL_DEBUG_INTR
	printk("end.\n");
#endif
}


/*
 * -------------------------------------------------------------------
 * Here ends the serial interrupt routines.
 * -------------------------------------------------------------------
 */

static void do_softint(struct work_struct *private_)
{
	struct tty_struct *tty;
	ser_info_t *info = container_of(private_, ser_info_t, work);

	tty = info->tty;
	if (!tty)
		return;

	if (test_and_clear_bit(RS_EVENT_WRITE_WAKEUP, &info->event)) {
		if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
		    tty->ldisc.write_wakeup)
			(tty->ldisc.write_wakeup)(tty);
		wake_up_interruptible(&tty->write_wait);
	}
}

/*
 * This routine is called from the scheduler tqueue when the interrupt
 * routine has signalled that a hangup has occurred.  The path of
 * hangup processing is:
 *
 * 	serial interrupt routine -> (scheduler tqueue) ->
 * 	do_serial_hangup() -> tty->hangup() -> rs_hangup()
 * 
 */
static void do_serial_hangup(struct work_struct *private_)
{
	struct tty_struct *tty;
	ser_info_t *info = container_of(private_, ser_info_t, hangup);
	
	tty = info->tty;
	if (tty)
		tty_hangup(tty);
}

static int startup(ser_info_t *info)
{
	unsigned long flags;
	int	retval=0;
	int	idx;
	struct serial_state *state= info->state;
	volatile smc_t		*smcp;
	volatile scc_t		*sccp;
	volatile smc_uart_t	*up;
	volatile scc_uart_t	*scup;
#ifdef CONFIG_PRxK
	immap_t *immap = (immap_t *)IMAP_ADDR;
#endif

	local_irq_save(flags);

	if (info->flags & ASYNC_INITIALIZED) {
		goto errout1;
	}

#ifdef maybe
	if (!state->port || !state->type) {
		if (info->tty)
			set_bit(TTY_IO_ERROR, &info->tty->flags);
		goto errout;
	}
#endif

#ifdef SERIAL_DEBUG_OPEN
#ifdef SERIAL_DEBUG_SCC
	if (PORT_IS_SCC(info->state->smc_scc_num))
#endif
	printk("starting up ttys%d (irq %d)...", info->line, state->irq);
#endif


#ifdef MODEM_CONTROL
	if (PORT_IS_SCC(info->state->smc_scc_num)) {
		info->MCR = 0;
		info->MSR = 0;
		// for full HW flow control
		immap->im_cpm.cp_pbdat &= ~(1 << (31 - CONFIG_RTS1_PIN));
		info->MCR |= UART_MCR_RTS | UART_MCR_DTR;
		if (info->tty->termios->c_cflag & CBAUD) {
			PORT_DTR1_DAT &= ~dtr1_pin;
		}
	}
#endif
	
	if (info->tty)
		clear_bit(TTY_IO_ERROR, &info->tty->flags);

	/*
	 * and set the speed of the serial port
	 */
	change_speed(info);
	
errout1:
	idx = PORT_NUM(info->state->smc_scc_num);
	if (PORT_IS_SCC(info->state->smc_scc_num)) {
		sccp = &cpmp->cp_scc[idx];
		scup = (scc_uart_t *)&cpmp->cp_dparam[state->port];
#ifdef CONFIG_PRxK
		scup->scc_genscc.scc_mrblr = RX_SCC_BUF_SIZE;
		scup->scc_maxidl = CONFIG_UART_MAXIDL_SCC1;
		sccp->scc_sccm |= (UART_SCCM_TX | UART_SCCM_RX | UART_SCCM_BSY);
#else
		scup->scc_genscc.scc_mrblr = RX_BUF_SIZE;
		scup->scc_maxidl = RX_BUF_SIZE;
		sccp->scc_sccm |= (UART_SCCM_TX | UART_SCCM_RX);
#endif
		sccp->scc_gsmrl |= (SCC_GSMRL_ENR | SCC_GSMRL_ENT);
	}
	else {
		smcp = &cpmp->cp_smc[idx];

		/* Enable interrupts and I/O.
		*/
#ifdef CONFIG_PRxK
		smcp->smc_smcm |= (SMCM_RX | SMCM_TX | SMCM_BSY);
#else
		smcp->smc_smcm |= (SMCM_RX | SMCM_TX);
#endif
		smcp->smc_smcmr |= (SMCMR_REN | SMCMR_TEN);

		/* We can tune the buffer length and idle characters
		 * to take advantage of the entire incoming buffer size.
		 * If mrblr is something other than 1, maxidl has to be
		 * non-zero or we never get an interrupt.  The maxidl
		 * is the number of character times we wait after reception
		 * of the last character before we decide no more characters
		 * are coming.
		 */
		up = (smc_uart_t *)&cpmp->cp_dparam[state->port];
		up->smc_mrblr = RX_BUF_SIZE;
		up->smc_maxidl = RX_BUF_SIZE;
		up->smc_brkcr = 1;	/* number of break chars */
	}

	info->flags |= ASYNC_INITIALIZED;
	local_irq_restore(flags);
	return 0;
	
errout:
	local_irq_restore(flags);
	return retval;
}

/*
 * This routine will shutdown a serial port; interrupts are disabled, and
 * DTR is dropped if the hangup on close termio flag is on.
 */
static void shutdown(ser_info_t * info)
{
	unsigned long	flags;
	struct serial_state *state;
	int		idx;
	volatile smc_t	*smcp;
	volatile scc_t	*sccp;

	if (!(info->flags & ASYNC_INITIALIZED))
		return;

	state = info->state;

#ifdef SERIAL_DEBUG_OPEN
#ifdef SERIAL_DEBUG_SCC
	if (PORT_IS_SCC(info->state->smc_scc_num))
#endif
	printk("Shutting down serial port %d (irq %d)....", info->line,
	       state->irq);
#endif
	
	local_irq_save(flags); /* Disable interrupts */

	idx = PORT_NUM(state->smc_scc_num);
	if (PORT_IS_SCC(state->smc_scc_num)) {
		sccp = &cpmp->cp_scc[idx];
		sccp->scc_gsmrl &= ~(SCC_GSMRL_ENR | SCC_GSMRL_ENT);
#ifdef CONFIG_SERIAL_CONSOLE
		/* We can't disable the transmitter if this is the
		 * system console.
		 */
		if ((state - rs_table) != CONFIG_SERIAL_CONSOLE_PORT)
#endif
#ifdef CONFIG_PRxK
			sccp->scc_sccm &= ~(UART_SCCM_TX | UART_SCCM_RX | UART_SCCM_BSY);
#else
			sccp->scc_sccm &= ~(UART_SCCM_TX | UART_SCCM_RX);
#endif
	}
	else {
		smcp = &cpmp->cp_smc[idx];

		/* Disable interrupts and I/O.
		*/
#ifdef CONFIG_PRxK
		smcp->smc_smcm &= ~(SMCM_RX | SMCM_TX | SMCM_BSY);
#else
		smcp->smc_smcm &= ~(SMCM_RX | SMCM_TX);
#endif
#ifdef CONFIG_SERIAL_CONSOLE
		/* We can't disable the transmitter if this is the
		 * system console.
		 */
		if ((state - rs_table) != CONFIG_SERIAL_CONSOLE_PORT)
#endif
			smcp->smc_smcmr &= ~(SMCMR_REN | SMCMR_TEN);
	}
	
#ifdef MODEM_CONTROL
	if (IS_BOARD_ACS1_KVM_TS1H) {
		if (PORT_IS_SCC(info->state->smc_scc_num)) {
			if (!info->tty || (info->tty->termios->c_cflag & HUPCL)) {
				PORT_DTR1_DAT |= dtr1_pin;
			}
		}
	}
#endif

	if (info->tty)
		set_bit(TTY_IO_ERROR, &info->tty->flags);

	info->flags &= ~ASYNC_INITIALIZED;
	local_irq_restore(flags);
}

/*
 * This routine is called to set the UART divisor registers to match
 * the specified baud rate for a serial port.
 */
static void change_speed(ser_info_t *info)
{
	int	baud_rate;
	unsigned cflag, cval, scval, prev_mode, new_mode;
	int	i, bits, sbits, idx;
	unsigned long	flags;
	struct serial_state *state;
	volatile smc_t	*smcp;
	volatile scc_t	*sccp;

	if (!info->tty || !info->tty->termios)
		return;
	cflag = info->tty->termios->c_cflag;

	state = info->state;

	/* Character length programmed into the mode register is the
	 * sum of: 1 start bit, number of data bits, 0 or 1 parity bit,
	 * 1 or 2 stop bits, minus 1.
	 * The value 'bits' counts this for us.
	 */
	cval = 0;
	scval = 0;

#ifdef MODEM_CONTROL
	if (IS_BOARD_ACS1_KVM_TS1H) {
		if (PORT_IS_SCC(info->state->smc_scc_num)) {
			scval = SCU_PSMR_FLC;
		}
	}
#endif

	/* byte size and parity */
	switch (cflag & CSIZE) {
	      case CS5: bits = 5; break;
	      case CS6: bits = 6; break;
	      case CS7: bits = 7; break;
	      case CS8: bits = 8; break;
	      /* Never happens, but GCC is too dumb to figure it out */
	      default:  bits = 8; break;
	}
	sbits = bits - 5;

	if (cflag & CSTOPB) {
		cval |= SMCMR_SL;	/* Two stops */
		scval |= SCU_PSMR_SL;
		bits++;
	}
	if (cflag & PARENB) {
		cval |= SMCMR_PEN;
		scval |= SCU_PSMR_PEN;
		bits++;
		if (!(cflag & PARODD)) {
			cval |= SMCMR_PM_EVEN;
			scval |= (SCU_PSMR_REVP | SCU_PSMR_TEVP);
		}
	}

	/* Determine divisor based on baud rate */
	i = cflag & CBAUD;
	if (i >= (sizeof(baud_table)/sizeof(int)))
		baud_rate = 9600;
	else
		baud_rate = baud_table[i];

#ifdef CONFIG_PRxK
	info->timeout = ((PORT_IS_SCC(info->state->smc_scc_num) ? TX_SCC_BUF_SIZE : TX_BUF_SIZE)*HZ*bits)/100;
#else
	info->timeout = (TX_BUF_SIZE*HZ*bits);
#endif
	info->timeout += HZ/50;		/* Add .02 seconds of slop */

#ifdef MODEM_CONTROL
	if (PORT_IS_SCC(info->state->smc_scc_num)) {

		/* CTS flow control flag and modem status interrupts */
		info->IER &= ~UART_IER_MSI;
		if (info->flags & ASYNC_HARDPPS_CD)
			info->IER |= UART_IER_MSI;
		if (cflag & CRTSCTS) {
			info->flags |= ASYNC_CTS_FLOW;
			info->IER |= UART_IER_MSI;
		} else
			info->flags &= ~ASYNC_CTS_FLOW;
		if (cflag & CLOCAL)
			info->flags &= ~ASYNC_CHECK_CD;
		else {
			info->flags |= ASYNC_CHECK_CD;
			info->IER |= UART_IER_MSI;
		}
		serial_out(info, UART_IER, info->IER);
	}
#endif

	/*
	 * Set up parity check flag
	 */
#define RELEVANT_IFLAG(iflag) (iflag & (IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK))

	info->read_status_mask = (BD_SC_EMPTY | BD_SC_OV);
	if (I_INPCK(info->tty))
		info->read_status_mask |= BD_SC_FR | BD_SC_PR;
	if (I_BRKINT(info->tty) || I_PARMRK(info->tty))
		info->read_status_mask |= BD_SC_BR;
	
	/*
	 * Characters to ignore
	 */
	info->ignore_status_mask = 0;
	if (I_IGNPAR(info->tty))
		info->ignore_status_mask |= BD_SC_PR | BD_SC_FR;
	if (I_IGNBRK(info->tty)) {
		info->ignore_status_mask |= BD_SC_BR;
		/*
		 * If we're ignore parity and break indicators, ignore 
		 * overruns too.  (For real raw support).
		 */
		if (I_IGNPAR(info->tty))
			info->ignore_status_mask |= BD_SC_OV;
	}
	/*
	 * !!! ignore all characters if CREAD is not set
	 */
	if ((cflag & CREAD) == 0)
		info->read_status_mask &= ~BD_SC_EMPTY;

	local_irq_save(flags);

	/* Start bit has not been added (so don't, because we would just
	 * subtract it later), and we need to add one for the number of
	 * stops bits (there is always at least one).
	 */
	bits++;
	idx = PORT_NUM(state->smc_scc_num);
	if (PORT_IS_SCC(state->smc_scc_num)) {
		sccp = &cpmp->cp_scc[idx];
		new_mode = (sbits << 12) | scval;
		prev_mode = sccp->scc_psmr;
		if (!(prev_mode & SCU_PSMR_PEN)) 
			/* If parity is disabled, mask out even/odd */
			prev_mode &= ~(SCU_PSMR_TPM|SCU_PSMR_RPM);
		if (prev_mode != new_mode)
			sccp->scc_psmr = new_mode;
	}
	else {
		smcp = &cpmp->cp_smc[idx];

		/* Set the mode register.  We want to keep a copy of the
		 * enables, because we want to put them back if they were
		 * present.
		 */
		prev_mode = smcp->smc_smcmr;
		new_mode = smcr_mk_clen(bits) | cval |  SMCMR_SM_UART;
		new_mode |= (prev_mode & (SMCMR_REN | SMCMR_TEN));
		if (!(prev_mode & SMCMR_PEN))
			/* If parity is disabled, mask out even/odd */
			prev_mode &= ~SMCMR_PM_EVEN;
		if (prev_mode != new_mode)
			smcp->smc_smcmr = new_mode;
	}

#ifdef CONFIG_NL1K
	m8xx_cpm_setbrg((state - rs_table) + 1, baud_rate);
#else
	m8xx_cpm_setbrg((state - rs_table), baud_rate);
//	connect_brg(state, i );
#endif

	local_irq_restore(flags);
}

static inline int waitBdpReady(ser_info_t *info, volatile cbd_t  *bdp)
{
	unsigned long orig_jiffies = jiffies;

	if (PORT_IS_SCC(info->state->smc_scc_num)) {
#ifdef SERIAL_DEBUG_SCC
		printk("waitBdpReady begin loop jiff=%lu...\n", jiffies);
#endif
	}

	while (bdp->cbd_sc & BD_SC_READY) {
		if (PORT_IS_SCC(info->state->smc_scc_num)) {
			current->state = TASK_INTERRUPTIBLE;
			schedule_timeout(5);
			if (signal_pending(current)) {
#ifdef SERIAL_DEBUG_SCC
					printk("waitBdpReady loop signal %ld\n", jiffies);
#endif
					return(0);
			}
			if (info->timeout &&
				time_after(jiffies, orig_jiffies + info->timeout)) {
#ifdef SERIAL_DEBUG_SCC
				printk("waitBdpReady loop timeout %ld\n", jiffies);
#endif
				return(0);
			}
		}
	}

#ifdef SERIAL_DEBUG_SCC
	if (PORT_IS_SCC(info->state->smc_scc_num)) {
		printk("put end loop jiff=%lu...\n", jiffies);
	}
#endif

	return(1);
}

static void rs_8xx_put_char(struct tty_struct *tty, unsigned char ch)
{
	ser_info_t *info = (ser_info_t *)tty->driver_data;
	volatile cbd_t	*bdp;
	unsigned char *cp;

	if (serial_paranoia_check(info, tty->name, "rs_put_char"))
		return;

	if (!tty)
		return;

#ifdef CONFIG_KGDB
        /* Try to let kgdb stub handle output on kgdb port. */
	if (info->line == KGDB_SER_IDX) {
		if (kgdb_output_string(&ch, 1));
			return;
	}
#endif

	if (!isDeviceConnected(info)) return;

	spin_lock(&info->lock);
	bdp = info->tx_cur;
	/* Get next BD.
	*/
	if (bdp->cbd_sc & BD_SC_WRAP)
		info->tx_cur = info->tx_bd_base;
	else
		info->tx_cur++;
	spin_unlock(&info->lock);

	if (!waitBdpReady(info, bdp)) return;

#ifdef CONFIG_PRxK
	if (PORT_IS_SCC(info->state->smc_scc_num)) {
		cp = info->tx_va_base + ((bdp - info->tx_bd_base) * TX_SCC_BUF_SIZE);
	} else 
#endif
		cp = info->tx_va_base + ((bdp - info->tx_bd_base) * TX_BUF_SIZE);
	*cp = ch;
	bdp->cbd_datlen = 1;
	bdp->cbd_sc |= BD_SC_READY;

#if 0
	/* Get next BD.
	*/
	if (bdp->cbd_sc & BD_SC_WRAP)
		bdp = info->tx_bd_base;
	else
		bdp++;

	info->tx_cur = (cbd_t *)bdp;
#endif

}

#ifdef CONFIG_PRxK
void line_is_idle(unsigned long data)
{
	ser_info_t *info = (ser_info_t*)data;

	info->event |= 1 << RS_EVENT_WRITE_WAKEUP;
	schedule_work(&info->work);
}

static int rs_8xx_framemode_write(struct tty_struct * tty, const unsigned char *buf, int count)
{
	immap_t *immap = (immap_t *)IMAP_ADDR;
	int c, ret = 0;
	int tx_buf_size;
	ser_info_t *info = (ser_info_t *)tty->driver_data;
	volatile cbd_t *bdp;
	unsigned char *cp;

#ifdef CONFIG_KGDB
	/* Try to let stub handle output. Returns true if it did. */
	if (kgdb_output_string(buf, count))
		return ret;
#endif

	if (serial_paranoia_check(info, tty->name, "rs_write"))
		return 0;
	if (!tty)
		return 0;

	//[RK]Oct/21/05
        if (!(PORT_IS_SCC(info->state->smc_scc_num))) {
                // console
                return(rs_8xx_write(tty,buf,count));
        }

	if (IS_BOARD_ACS1_KVM_TS1H &&
	    (PORT_IS_SCC(info->state->smc_scc_num))) {
		if (info->media && (info->flags & ASYNC_LEGACY_RTS)) {
			// RS485 Half Duplex
			if (!(immap->im_cpm.cp_scc[PORT_NUM(info->state->smc_scc_num)].scc_sccs & 0x01)) { 
				/* The line is not idle */
				init_timer(&wait_idle_line);
				wait_idle_line.function = line_is_idle;
				wait_idle_line.data = (unsigned long)info;
				wait_idle_line.expires = HZ / 100; //10ms
				add_timer(&wait_idle_line);
				return 0;
			}
		}
		if (!info->media && info->hw_stopped) { // RS232 - CTS ctrl
			info->flags |= TX_WAKEUP;
			if (!info->bdtx &&
			    (serial_in(info,UART_MCR) & UART_MCR_RTS)) {
				local_irq_disable();
				info->MCR &= ~UART_MCR_RTS; // RTS Off
				serial_out(info,UART_MCR, info->MCR);
				local_irq_enable();
			}
			return 0;
		}
	}

	if (info->frame_cfg.mode == FRAMEMODE_RTU) {
		if (rtuend.tv_usec || rtuend.tv_usec) {
			struct timeval time;
			long interval;

			do_gettimeofday(&time);
			if ((interval = time.tv_sec - rtuend.tv_sec) <= 0) {
				if (!interval) {
					if ((time.tv_usec - rtuend.tv_usec) < 0) {
						// waiting 3,5 chars time...
						return 0;
					}
				} else {
					// waiting 3,5 chars time...
					return 0;
				}
			}
		}
	}

	bdp = info->tx_cur;

	tx_buf_size = (PORT_IS_SCC(info->state->smc_scc_num) ? TX_SCC_BUF_SIZE : TX_BUF_SIZE);

	if (bdp->cbd_sc & BD_SC_READY) {
		info->flags |= TX_WAKEUP;
		return 0;
	}

	if ( count > tx_buf_size ) {
		printk("framemode_write(): tx buffer is too short\n");
		return 0;
	}

	cp = info->tx_va_base + ((bdp - info->tx_bd_base) * tx_buf_size);

	if ( info->frame_cfg.txcount == 0 ) {
		short aux;

		memcpy( (char *) &aux, buf, 2);
		ret += 2;
		count -= 2;
		buf += 2;

		if ( info->frame_cfg.mode == FRAMEMODE_ASCII ) {
			//* (char *) __va(bdp->cbd_bufaddr) = 0x3a;   // (:)
			*cp = 0x3a;   // (:)
			info->frame_cfg.txcount ++;
			bdp->cbd_datlen = aux - 1;
		} else {
			bdp->cbd_datlen = aux - 2;
		}
	}
	
	c = count;

	memcpy((void *) (cp + info->frame_cfg.txcount), buf, c);

	info->frame_cfg.txcount += c;
	ret += c;

	if ( info->frame_cfg.txcount == bdp->cbd_datlen) {
		if ( info->frame_cfg.mode == FRAMEMODE_ASCII ) {
			* (cp+info->frame_cfg.txcount) = 0x0d;
			* (cp+info->frame_cfg.txcount+1) = 0x0a;
			bdp->cbd_datlen += 2;
		}
		if (PORT_IS_SCC(info->state->smc_scc_num)) {
			if (info->flags & ASYNC_LEGACY_RTS) {
				if (!info->bdtx) {
					local_irq_disable();
					info->bdtx = (cbd_t *) bdp;
					info->MCR |= UART_MCR_RTS; // RTS ON
					serial_out(info,UART_MCR, info->MCR);
					bdp->cbd_sc |= (BD_SC_READY | BD_SC_CR);
					local_irq_enable();
				}
			} else {
                		bdp->cbd_sc |= (BD_SC_READY | BD_SC_CR);
			}
		} else {
			bdp->cbd_sc |= BD_SC_READY;
		}

		bdp->cbd_sc &= ~BD_SC_P;
		info->frame_cfg.txcount = 0;
		info->frame_cfg.tx_stat.success++;

		/* Get next BD. */
		if (bdp->cbd_sc & BD_SC_WRAP)
			bdp = info->tx_bd_base;
		else
			bdp++;
		info->tx_cur = (cbd_t *)bdp;
	}

	return ret;
}

#endif

static int rs_8xx_write(struct tty_struct * tty, 
		    const unsigned char *buf, int count)
{
	int	c, ret = 0, tx_buf_size=TX_BUF_SIZE;
	ser_info_t *info = (ser_info_t *)tty->driver_data;
	volatile cbd_t *bdp;
	unsigned char *cp;

#ifdef CONFIG_KGDB
        /* Try to let kgdb stub handle output on kgdb port.
	 *  Returns character count if successful, 0 otherwise. */
	if (info->line == KGDB_SER_IDX) {
		if ((ret = kgdb_output_string(buf, count)));
			return ret;
	}
#endif

	if (serial_paranoia_check(info, tty->name, "rs_write"))
		return 0;

	if (!tty) 
		return 0;

	if (!isDeviceConnected(info)) return(count);

#ifdef CONFIG_PRxK
	if (PORT_IS_SCC(info->state->smc_scc_num)) {
		tx_buf_size = TX_SCC_BUF_SIZE;
		if (info->media && (info->flags & ASYNC_LEGACY_RTS)) {
			// RS485 Half Duplex
			if (!(immap->im_cpm.cp_scc[PORT_NUM(info->state->smc_scc_num)].scc_sccs & 0x01)) {
				/* The line is not idle */
				init_timer(&wait_idle_line);
				wait_idle_line.function = line_is_idle;
				wait_idle_line.data = (unsigned long)info;
				wait_idle_line.expires = HZ / 100; //10ms
				add_timer(&wait_idle_line);
				return 0;
			}
		}
		if (!info->media && info->hw_stopped && (info->flags & ASYNC_LEGACY_CTS)) { 
			// RS232 - CTS ctrl
			info->flags |= TX_WAKEUP;
			if (!info->bdtx && (serial_in(info,UART_MCR) & UART_MCR_RTS)) {
				local_irq_disable();
				info->MCR &= ~UART_MCR_RTS; // RTS Off
				serial_out(info,UART_MCR, info->MCR);
				local_irq_enable();
			}
			return 0;
		}
	}

#endif

	spin_lock(&info->lock);
	bdp = info->tx_cur;

	while (1) {
		c = MIN(count, (PORT_IS_SCC(info->state->smc_scc_num) ? TX_SCC_BUF_SIZE : TX_BUF_SIZE));

		if (c <= 0)
			break;

		if (bdp->cbd_sc & BD_SC_READY) {
			info->flags |= TX_WAKEUP;
			break;
		}

		cp = info->tx_va_base + ((bdp - info->tx_bd_base) * tx_buf_size);

		memcpy((void *) cp, buf, c);

		bdp->cbd_datlen = c;
#ifdef CONFIG_PRxK
		if (PORT_IS_SCC(info->state->smc_scc_num)) {
			if (info->flags & ASYNC_LEGACY_RTS) {
				if (!info->bdtx) {
					local_irq_disable();
					info->bdtx = (cbd_t *) bdp;
					info->MCR |= UART_MCR_RTS; // RTS ON
					serial_out(info,UART_MCR, info->MCR);
					bdp->cbd_sc |= (BD_SC_READY | BD_SC_CR);
					local_irq_enable();
				}
			} else {
				bdp->cbd_sc |= (BD_SC_READY | BD_SC_CR);
			}
		} else {
			bdp->cbd_sc |= BD_SC_READY;
		}

#else
		bdp->cbd_sc |= BD_SC_READY;
#endif

		buf += c;
		count -= c;
		ret += c;

		/* Get next BD.
		*/
		if (bdp->cbd_sc & BD_SC_WRAP)
			bdp = info->tx_bd_base;
		else
			bdp++;
		info->tx_cur = (cbd_t *)bdp;
	}

	spin_unlock(&info->lock);
	return ret;
}


static int rs_8xx_write_room(struct tty_struct *tty)
{
	ser_info_t *info = (ser_info_t *)tty->driver_data;
	int	ret;

	if (serial_paranoia_check(info, tty->name, "rs_write_room"))
		return 0;

	if ((info->tx_cur->cbd_sc & BD_SC_READY) == 0) {
		info->flags &= ~TX_WAKEUP;
#ifdef CONFIG_PRxK
		ret = (PORT_IS_SCC(info->state->smc_scc_num) ? TX_SCC_BUF_SIZE : TX_BUF_SIZE);
#else
		ret = TX_BUF_SIZE;
#endif
	}
	else {
		info->flags |= TX_WAKEUP;
		ret = 0;
	}
	return ret;
}

/* I could track this with transmit counters....maybe later.
*/
static int rs_8xx_chars_in_buffer(struct tty_struct *tty)
{
	ser_info_t *info = (ser_info_t *)tty->driver_data;
				
	if (serial_paranoia_check(info, tty->name, "rs_chars_in_buffer"))
		return 0;
	return 0;
}

static void rs_8xx_flush_buffer(struct tty_struct *tty)
{
	ser_info_t *info = (ser_info_t *)tty->driver_data;
				
	if (serial_paranoia_check(info, tty->name, "rs_flush_buffer"))
		return;

	/* There is nothing to "flush", whatever we gave the CPM
	 * is on its way out.
	 */
	wake_up_interruptible(&tty->write_wait);
	if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
	    tty->ldisc.write_wakeup)
		(tty->ldisc.write_wakeup)(tty);
	info->flags &= ~TX_WAKEUP;
}

/*
 * This function is used to send a high-priority XON/XOFF character to
 * the device
 */
static void rs_8xx_send_xchar(struct tty_struct *tty, char ch)
{
	volatile cbd_t	*bdp;
	unsigned char *cp;
	unsigned int tx_buf_size=TX_BUF_SIZE;

	ser_info_t *info = (ser_info_t *)tty->driver_data;

	if (serial_paranoia_check(info, tty->name, "rs_send_char"))
		return;

#ifdef CONFIG_PRxK
	spin_lock(&info->lock);
	bdp = info->tx_cur;
	if (PORT_IS_SCC(info->state->smc_scc_num))
		tx_buf_size = TX_SCC_BUF_SIZE;
	if (bdp->cbd_sc & BD_SC_WRAP)
		info->tx_cur = info->tx_bd_base;
	else
		info->tx_cur++;
	spin_unlock(&info->lock);
#else
	bdp = info->tx_cur;
#endif

	if (!waitBdpReady(info, bdp)) return;

	cp = info->tx_va_base + ((bdp - info->tx_bd_base) * tx_buf_size);
	*cp = ch;

	bdp->cbd_datlen = 1;
#ifdef CONFIG_PRxK
	if (PORT_IS_SCC(info->state->smc_scc_num))
		bdp->cbd_sc |= BD_SC_CR;
#endif
	bdp->cbd_sc |= BD_SC_READY;

#if 0
	/* Get next BD.
	*/
	if (bdp->cbd_sc & BD_SC_WRAP)
		bdp = info->tx_bd_base;
	else
		bdp++;

	info->tx_cur = (cbd_t *)bdp;
#endif
}

/*
 * ------------------------------------------------------------
 * rs_throttle()
 * 
 * This routine is called by the upper-layer tty layer to signal that
 * incoming characters should be throttled.
 * ------------------------------------------------------------
 */
static void rs_8xx_throttle(struct tty_struct * tty)
{
	ser_info_t *info = (ser_info_t *)tty->driver_data;
#ifdef SERIAL_DEBUG_THROTTLE
	char	buf[64];
	
	printk("throttle %s: %d....\n", _tty_name(tty, buf),
	       tty->ldisc.chars_in_buffer(tty));
#endif

	if (serial_paranoia_check(info, tty->name, "rs_throttle"))
		return;
	
	if (I_IXOFF(tty))
		rs_8xx_send_xchar(tty, STOP_CHAR(tty));

#ifdef MODEM_CONTROL
	if (PORT_IS_SCC(info->state->smc_scc_num)) {
		if (tty->termios->c_cflag & CRTSCTS) {
			info->MCR &= ~UART_MCR_RTS;
		}
		serial_out(info, UART_MCR, info->MCR);
	}
#endif
}

static void rs_8xx_unthrottle(struct tty_struct * tty)
{
	ser_info_t *info = (ser_info_t *)tty->driver_data;
#ifdef SERIAL_DEBUG_THROTTLE
	char	buf[64];
	
	printk("unthrottle %s: %d....\n", _tty_name(tty, buf),
	       tty->ldisc.chars_in_buffer(tty));
#endif

	if (serial_paranoia_check(info, tty->name, "rs_unthrottle"))
		return;
	
	if (I_IXOFF(tty)) {
		if (info->x_char)
			info->x_char = 0;
		else
			rs_8xx_send_xchar(tty, START_CHAR(tty));
	}
#ifdef MODEM_CONTROL
	if (PORT_IS_SCC(info->state->smc_scc_num)) {
		if (tty->termios->c_cflag & CRTSCTS) {
			info->MCR |= UART_MCR_RTS;
		}
		serial_out(info, UART_MCR, info->MCR);
	}

#endif
}

/*
 * ------------------------------------------------------------
 * rs_ioctl() and friends
 * ------------------------------------------------------------
 */

#ifdef maybe
/*
 * get_lsr_info - get line status register info
 *
 * Purpose: Let user call ioctl() to get info when the UART physically
 * 	    is emptied.  On bus types like RS485, the transmitter must
 * 	    release the bus after transmitting. This must be done when
 * 	    the transmit shift register is empty, not be done when the
 * 	    transmit holding register is empty.  This functionality
 * 	    allows an RS485 driver to be written in user space. 
 */
static int get_lsr_info(struct async_struct * info, unsigned int *value)
{
	unsigned char status;
	unsigned int result;

	local_irq_disable();
	status = serial_in(info, UART_LSR);
	local_irq_enable();
	result = ((status & UART_LSR_TEMT) ? TIOCSER_TEMT : 0);
	return put_user(result,value);
}
#endif
static int rs_tiocmget(struct tty_struct *tty, struct file *file)
{
#ifdef MODEM_CONTROL
	unsigned char control, status;
	ser_info_t *info = (ser_info_t *)tty->driver_data;

	if (serial_paranoia_check(info, tty->name, "rs_ioctl"))
       	 return -ENODEV;
	if (tty->flags & (1 << TTY_IO_ERROR))
       	  return -EIO;

	if (IS_BOARD_ACS1_KVM_TS1H &&
	    (PORT_IS_SCC(info->state->smc_scc_num))) {

		control = info->MCR;
		local_irq_disable();
		status = serial_in(info, UART_MSR);
		local_irq_enable();
		return  ((control & UART_MCR_RTS) ? TIOCM_RTS : 0)
			| ((control & UART_MCR_DTR) ? TIOCM_DTR : 0)
#ifdef TIOCM_OUT1
			| ((control & UART_MCR_OUT1) ? TIOCM_OUT1 : 0)
			| ((control & UART_MCR_OUT2) ? TIOCM_OUT2 : 0)
#endif
			| ((status  & UART_MSR_DCD) ? TIOCM_CAR : 0)
			| ((status  & UART_MSR_RI) ? TIOCM_RNG : 0)
			| ((status  & UART_MSR_DSR) ? TIOCM_DSR : 0)
			| ((status  & UART_MSR_CTS) ? TIOCM_CTS : 0);
	}
	return 0;
#endif
}

static int rs_tiocmset(struct tty_struct *tty, struct file *file,
               unsigned int set, unsigned int clear)
{
       ser_info_t *info = (ser_info_t *)tty->driver_data;

       if (serial_paranoia_check(info, tty->name, "rs_ioctl"))
               return -ENODEV;
       if (tty->flags & (1 << TTY_IO_ERROR))
               return -EIO;

#ifdef MODEM_CONTROL
	if (IS_BOARD_ACS1_KVM_TS1H) {
		local_irq_disable();
		if (set & TIOCM_RTS)
			info->MCR |= UART_MCR_RTS;
		if (set & TIOCM_DTR)
			info->MCR |= UART_MCR_DTR;
		if (clear & TIOCM_RTS)
			info->MCR &= ~UART_MCR_RTS;
		if (clear & TIOCM_DTR)
			info->MCR &= ~UART_MCR_DTR;

		serial_out(info, UART_MCR, info->MCR);
	}
#endif
	return 0;
}

static inline int isDeviceConnected(ser_info_t *info)
{
	unsigned char status;

	if (PORT_IS_SCC(info->state->smc_scc_num) &&
		(info->flags & ASYNC_CTS_FLOW)) {
		local_irq_disable();
		status = serial_in(info, UART_MSR);
		local_irq_enable();
		return(status  & (UART_MSR_DCD|UART_MSR_RI|UART_MSR_DSR|UART_MSR_CTS));
	}
	return(1);
}

static int get_modem_info(ser_info_t *info, unsigned int *value)
{
	unsigned int result = 0;
#ifdef modem_control
	unsigned char control, status;
	//[RK]Jun/22/04 - ACS SW for TS100Rev2
	if (IS_BOARD_ACS1_KVM_TS1H &&  
		(PORT_IS_SCC(info->state->smc_scc_num))) {
		control = info->MCR;
		local_irq_disable();
		status = serial_in(info, UART_MSR);
		local_irq_enable();
		result =  ((control & UART_MCR_RTS) ? TIOCM_RTS : 0)
			| ((control & UART_MCR_DTR) ? TIOCM_DTR : 0)
#ifdef TIOCM_OUT1
			| ((control & UART_MCR_OUT1) ? TIOCM_OUT1 : 0)
			| ((control & UART_MCR_OUT2) ? TIOCM_OUT2 : 0)
#endif
			| ((status  & UART_MSR_DCD) ? TIOCM_CAR : 0)
			| ((status  & UART_MSR_RI) ? TIOCM_RNG : 0)
			| ((status  & UART_MSR_DSR) ? TIOCM_DSR : 0)
			| ((status  & UART_MSR_CTS) ? TIOCM_CTS : 0);
	}
#endif
	return put_user(result,value);
}

static int set_modem_info(ser_info_t *info, unsigned int cmd,
			  unsigned int *value)
{
	int error;
	unsigned int arg;

	error = get_user(arg,(unsigned int *) value);
	if (error) return error;

#ifdef modem_control
	//[RK]Jun/22/04 - ACS SW for TS100Rev2
	if (IS_BOARD_ACS1_KVM_TS1H) {   
		switch (cmd) {
		case TIOCMBIS: 
			if (arg & TIOCM_RTS)
				info->MCR |= UART_MCR_RTS;
			if (arg & TIOCM_DTR)
				info->MCR |= UART_MCR_DTR;
#ifdef TIOCM_OUT1
			if (arg & TIOCM_OUT1)
				info->MCR |= UART_MCR_OUT1;
			if (arg & TIOCM_OUT2)
				info->MCR |= UART_MCR_OUT2;
#endif
			break;
		case TIOCMBIC:
			if (arg & TIOCM_RTS)
				info->MCR &= ~UART_MCR_RTS;
			if (arg & TIOCM_DTR)
				info->MCR &= ~UART_MCR_DTR;
#ifdef TIOCM_OUT1
			if (arg & TIOCM_OUT1)
				info->MCR &= ~UART_MCR_OUT1;
			if (arg & TIOCM_OUT2)
				info->MCR &= ~UART_MCR_OUT2;
#endif
			break;
		case TIOCMSET:
			info->MCR = ((info->MCR & ~(UART_MCR_RTS |
#ifdef TIOCM_OUT1
						    UART_MCR_OUT1 |
						    UART_MCR_OUT2 |
#endif
						    UART_MCR_DTR))
				     | ((arg & TIOCM_RTS) ? UART_MCR_RTS : 0)
#ifdef TIOCM_OUT1
				     | ((arg & TIOCM_OUT1) ? UART_MCR_OUT1 : 0)
				     | ((arg & TIOCM_OUT2) ? UART_MCR_OUT2 : 0)
#endif
				     | ((arg & TIOCM_DTR) ? UART_MCR_DTR : 0));
			break;
		default:
			return -EINVAL;
		}
		local_irq_disable();
		serial_out(info, UART_MCR, info->MCR);
		local_irq_enable();
	}
#endif	/* modem_control */
	return 0;
}

/* Sending a break is a two step process on the SMC/SCC.  It is accomplished
 * by sending a STOP TRANSMIT command followed by a RESTART TRANSMIT
 * command.  We take advantage of the begin/end functions to make this
 * happen.
 */
static ushort	smc_chan_map[] = {
	CPM_CR_CH_SMC1,
	CPM_CR_CH_SMC2
};

static ushort	scc_chan_map[] = {
	CPM_CR_CH_SCC1,
	CPM_CR_CH_SCC2,
	CPM_CR_CH_SCC3,
	CPM_CR_CH_SCC4
};

static void begin_break(ser_info_t *info)
{
	volatile cpm8xx_t *cp;
	ushort	chan;
	int	idx;

	cp = cpmp;

	idx = PORT_NUM(info->state->smc_scc_num);
	if (PORT_IS_SCC(info->state->smc_scc_num))
		chan = scc_chan_map[idx];
	else
		chan = smc_chan_map[idx];
	cp->cp_cpcr = mk_cr_cmd(chan, CPM_CR_STOP_TX) | CPM_CR_FLG;
	while (cp->cp_cpcr & CPM_CR_FLG);
}

static void end_break(ser_info_t *info)
{
	volatile cpm8xx_t *cp;
	ushort	chan;
	int	idx;

	cp = cpmp;

	idx = PORT_NUM(info->state->smc_scc_num);
	if (PORT_IS_SCC(info->state->smc_scc_num))
		chan = scc_chan_map[idx];
	else
		chan = smc_chan_map[idx];
	cp->cp_cpcr = mk_cr_cmd(chan, CPM_CR_RESTART_TX) | CPM_CR_FLG;
	while (cp->cp_cpcr & CPM_CR_FLG);
}

/*
 * This routine sends a break character out the serial port.
 */
static void send_break(ser_info_t *info, int duration)
{
#ifdef CONFIG_PRxK
// mpeccin:07Fev2003
// The idea of starting and stopping break with the functions begin_ and
// end_break() seems to me not correct for the MPC860 because once
// we STOP_TX the SCC automatically inserts the number of break characters
// specified by its BRKCR parameter.
// I tried with a big BRKCR rely on the RESTART_TX to stop sending break, but
// found out that SCC will not in fact restart TX until all BRKCR characters
// are sent.
// So, the only strategy is to program BRKCR in a case by case basis...
//
//       (number of chars for 1s break) * (number of seconds or fraction desired)
//
// Bugfix 4873 mp[Sep/2005]

    int nchars = (baud_table[info->tty->termios->c_cflag & CBAUD]/10) * duration / HZ;
    //note that we first multiply and then divide...
    
	((scc_uart_t *)&cpmp->cp_dparam[info->state->port])->scc_brkcr = nchars ? nchars : 1;
#endif
	current->state = TASK_INTERRUPTIBLE;
#ifdef SERIAL_DEBUG_SEND_BREAK
	printk("rs_send_break(%d) jiff=%lu...\n", duration, jiffies);
#endif
	begin_break(info);
	schedule_timeout(duration);
	end_break(info);
#ifdef SERIAL_DEBUG_SEND_BREAK
	printk("done jiffies=%lu\n", jiffies);
#endif
}

#ifdef CONFIG_PRxK
static int cyc_set_command(ser_info_t *info, unsigned int * arg)
{
	immap_t *immap = (immap_t *)IMAP_ADDR;
	unsigned int command, aux, media;

	aux = get_user(command,(unsigned int *) arg);
	if (aux) return aux;

	aux = tty_check_change(info->tty);
	if (aux) return aux;

	if ((command & 0xffff0000) == CYC_GET_MEDIA) {
		//get media value
		media = info->media;
		if (info->flags & ASYNC_LEGACY_RTS) {
			media |= 2;
		}
		if (info->flags & ASYNC_LEGACY_CTS) {
			media |= 8;
		}
		aux = put_user(media, (int *)arg);
		return 0;
	}

	if ((command & 0xffff0000) == CYC_SET_MEDIA) {
		media = command & 0xffff;

		if (media & 1) { // RS485
			info->media = media & 9;
		} else {
			info->media = 0; // RS232
		}
		tty_wait_until_sent(info->tty, 0);
		immap->im_ioport.iop_pcdat |= (1 << 9); // termination OFF
		info->terminator = 0;

		if (IS_BOARD_TS1H || IS_BOARD_ACS1) {
			/* PA10=retval:31, PA9=retval:30, PA8=retval:29 */
			/* AB_SEL, C_SEL, D_SEL, respectively */
			immap->im_ioport.iop_padat &=  ~(7 << 5);

			if (media & 1) { // RS485
				immap->im_ioport.iop_padat |=  (media & 7) << 5;
				if (!(info->media & 8)) {
					/* PC6=retval:28 - termination resistor */
					info->terminator = 1;
					immap->im_ioport.iop_pcdat &= ~(1 << 9); // termination OFF
				}
			}
		}

		info->flags &= ~(ASYNC_LEGACY_RTS | ASYNC_LEGACY_CTS);
		info->hw_stopped = 0;

		if (!info->media && ((media & 0x0a) == 0x0a)) {
			// RS232 - RTS/CTS control
			info->flags |= ASYNC_LEGACY_RTS | ASYNC_LEGACY_CTS;
		} else {
			if (media & 0x2) { // Halfd - RTS ctrl
				info->flags |= ASYNC_LEGACY_RTS;
				info->flags |= ASYNC_LEGACY_RTS;
			}
		}

		if (info->flags & ASYNC_LEGACY_RTS) {
			// do RTS down
			info->MCR &= ~UART_MCR_RTS;
			local_irq_disable();
			serial_out(info, UART_MCR, info->MCR);
			local_irq_enable();

			if (info->flags & ASYNC_LEGACY_CTS) {
				info->IER |= UART_IER_MSI;
				immap->im_ioport.iop_pcso &= ~(0x10);
				local_irq_disable();
				serial_out(info, UART_IER, info->IER);
				local_irq_enable();
				aux = serial_in(info, UART_MSR);
				if (aux & UART_MSR_CTS) {
					info->hw_stopped = 1;
				}
			}
		} else {
			// Full  ==> do RTS UP
			info->MCR |= UART_MCR_RTS;
			if (!(info->flags & (ASYNC_CHECK_CD|ASYNC_CTS_FLOW))) {
				info->IER &= ~UART_IER_MSI;
			}
			immap->im_ioport.iop_pcso |= 0x10;
			local_irq_disable();
			serial_out(info, UART_MCR, info->MCR);
			serial_out(info, UART_IER, info->IER);
			local_irq_enable();
		}
		return 0;
	}
	return -EINVAL;
}

#endif

static int rs_8xx_ioctl(struct tty_struct *tty, struct file * file,
		    unsigned int cmd, unsigned long arg)
{
	immap_t *immap = (immap_t *)IMAP_ADDR;
	int error;
	ser_info_t *info = (ser_info_t *)tty->driver_data;
    int retval;
	struct async_icount cnow;	/* kernel counter temps */
	struct async_icount cprev;	/* kernel counter temps */
	struct serial_icounter_struct *p_cuser;	/* user space */

	if (serial_paranoia_check(info, tty->name, "rs_ioctl"))
		return -ENODEV;
#ifdef CONFIG_PRxK
	if ((cmd != TIOCMIWAIT) && (cmd != TIOCGICOUNT) && (cmd != TIOCSERCYC)) {
		if (tty->flags & (1 << TTY_IO_ERROR))
		    return -EIO;
	}
#else
	if ((cmd != TIOCMIWAIT) && (cmd != TIOCGICOUNT)) {
		if (tty->flags & (1 << TTY_IO_ERROR))
		    return -EIO;
	}
#endif	
	switch (cmd) {
		case TCSBRK:	/* SVID version: non-zero arg --> no break */
			retval = tty_check_change(tty);
			if (retval)
				return retval;
			tty_wait_until_sent(tty, 0);
			if (signal_pending(current))
				return -EINTR;
			if (!arg) {
				send_break(info, HZ/4);	/* 1/4 second */
				if (signal_pending(current))
					return -EINTR;
			}
			return 0;
		case TCSBRKP:	/* support for POSIX tcsendbreak() */
			retval = tty_check_change(tty);
			if (retval)
				return retval;
			tty_wait_until_sent(tty, 0);
			if (signal_pending(current))
				return -EINTR;
			send_break(info, arg ? arg*(HZ/10) : HZ/4);
			if (signal_pending(current))
				return -EINTR;
			return 0;
		case TIOCSERCONFIG:
			return 0;
		case TIOCSBRK:
			retval = tty_check_change(tty);
			if (retval)
				return retval;
			tty_wait_until_sent(tty, 0);
			begin_break(info);
			return 0;
		case TIOCCBRK:
			retval = tty_check_change(tty);
			if (retval)
				return retval;
			end_break(info);
			return 0;
		case TIOCGSOFTCAR:
			return put_user(C_CLOCAL(tty) ? 1 : 0, (int *) arg);
		case TIOCSSOFTCAR:
			error = get_user(arg, (unsigned int *) arg);
			if (error)
				return error;
			tty->termios->c_cflag =
				((tty->termios->c_cflag & ~CLOCAL) |
				 (arg ? CLOCAL : 0));
			return 0;
		case TIOCMGET:
			return get_modem_info(info, (unsigned int *) arg);
		case TIOCMBIS:
		case TIOCMBIC:
		case TIOCMSET:
			return set_modem_info(info, cmd, (unsigned int *) arg);
#ifdef maybe
		case TIOCSERGETLSR: /* Get line status register */
			return get_lsr_info(info, (unsigned int *) arg);
#endif
		/*
		 * Wait for any of the 4 modem inputs (DCD,RI,DSR,CTS) to change
		 * - mask passed in arg for lines of interest
 		 *   (use |'ed TIOCM_RNG/DSR/CD/CTS for masking)
		 * Caller should use TIOCGICOUNT to see which one it was
		 */
		 case TIOCMIWAIT:
#ifdef MODEM_CONTROL
		if (IS_BOARD_ACS1_KVM_TS1H && (PORT_IS_SCC(info->state->smc_scc_num))) {
			
			local_irq_disable();
			/* note the counters on entry */
			cprev = info->state->icount;
			local_irq_enable();
			while (1) {
				interruptible_sleep_on(&info->delta_msr_wait);
				/* see if a signal did it */
				if (signal_pending(current))
					return -ERESTARTSYS;
				local_irq_disable();
				cnow = info->state->icount; /* atomic copy */
				local_irq_enable();
				if (cnow.rng == cprev.rng && cnow.dsr == cprev.dsr && 
				    cnow.dcd == cprev.dcd && cnow.cts == cprev.cts)
					return -EIO; /* no change => error */
				if ( ((arg & TIOCM_RNG) && (cnow.rng != cprev.rng)) ||
				     ((arg & TIOCM_DSR) && (cnow.dsr != cprev.dsr)) ||
				     ((arg & TIOCM_CD)  && (cnow.dcd != cprev.dcd)) ||
				     ((arg & TIOCM_CTS) && (cnow.cts != cprev.cts)) ) {
					return 0;
				}
				cprev = cnow;
			}
			/* NOTREACHED */
		} else {
			return 0;
		}
#else
			return 0;
#endif

		/* 
		 * Get counter of input serial line interrupts (DCD,RI,DSR,CTS)
		 * Return: write counters to the user passed counter struct
		 * NB: both 1->0 and 0->1 transitions are counted except for
		 *     RI where only 0->1 is counted.
		 */
		case TIOCGICOUNT:
			local_irq_disable();
			cnow = info->state->icount;
			local_irq_enable();
			p_cuser = (struct serial_icounter_struct *) arg;
			error = put_user(cnow.cts, &p_cuser->cts);
			if (error) return error;
			error = put_user(cnow.dsr, &p_cuser->dsr);
			if (error) return error;
			error = put_user(cnow.rng, &p_cuser->rng);
			if (error) return error;
			error = put_user(cnow.dcd, &p_cuser->dcd);
			if (error) return error;
			return 0;

#ifdef CONFIG_PRxK
		case FRAMEMODESCFG:     // Frame-Mode Support
		if (PORT_IS_SCC(info->state->smc_scc_num)) {
			struct      serial_state *ser = info->state;
			volatile    cpm8xx_t     *cp = cpmp;
			volatile    scc_uart_t   *sup;
			volatile    scc_t        *scp;
			ushort chan = scc_chan_map[PORT_NUM(info->state->smc_scc_num)];

			scp = &immap->im_cpm.cp_scc[PORT_NUM(info->state->smc_scc_num)];

			sup = (scc_uart_t *)&cp->cp_dparam[ser->port];

			// Getting the configuration structure
			error = copy_from_user((void *)&info->frame_cfg,
					(void *) arg, sizeof(struct frame_mode_cfg));
			if (error != 0) {
				printk("    error on copy.from.user\n");
				return error;
			}

			if ( info->frame_cfg.status == FRAMEMODE_ON ) {
				// Resetting framemode configuration
				info->frame_cfg.txcount = 0;
				info->frame_cfg.rxcount = 0;
				info->frame_cfg.frame_started = 0;
				// Changing the write routine...
				tty->driver->write = rs_8xx_framemode_write;
				scp->scc_gsmrl &= ~SCC_GSMRL_ENR;
				// ...and SCC parameters
				if (info->frame_cfg.mode == FRAMEMODE_RTU) {
					sup->scc_maxidl = FRAMEMODE_IDLEWORDS;
					sup->scc_char1 = 0x8000;
				} else {   // ascii
					sup->scc_maxidl = 0;
					sup->scc_char1 = 0x000A;
				}
				cp->cp_cpcr = mk_cr_cmd(chan, CPM_CR_INIT_RX) | CPM_CR_FLG;
				while (cp->cp_cpcr & CPM_CR_FLG);
				scp->scc_gsmrl |= SCC_GSMRL_ENR;
			} else {
				// Changing the write routine...
				tty->driver->write = rs_8xx_write;
				scp->scc_gsmrl &= ~SCC_GSMRL_ENR;
				// ...and SCC parameters
				sup->scc_maxidl = CONFIG_UART_MAXIDL_SCC1;
				sup->scc_char1 = 0x8000;
				cp->cp_cpcr = mk_cr_cmd(chan, CPM_CR_INIT_RX) | CPM_CR_FLG;
				while (cp->cp_cpcr & CPM_CR_FLG);
				scp->scc_gsmrl |= SCC_GSMRL_ENR;
			}
			//because CPM_CR_INIT_RX causes next buffer descriptor to be scc_rbase
			info->rx_cur = info->rx_bd_base;
		}
		return 0;

		case FRAMEMODEGCFG:
			if (PORT_IS_SCC(info->state->smc_scc_num)) {
				// Getting the configuration structure
				error = copy_to_user((void *) arg, (void *)&info->frame_cfg,
						sizeof(struct frame_mode_cfg));
				if (error != 0) {
					printk("uart.c:ioctl() - error on copy.from.user\n");
					return error;
				}
			}
			break;

		case TIOCSERCYC :
			if (PORT_IS_SCC(info->state->smc_scc_num)) {
				return cyc_set_command(info,(unsigned int *)arg);
			} else {
				return -EIO;
			}
#endif

		default:
			return -ENOIOCTLCMD;
		}
	return 0;
}

#if 0
static void print_termios(struct ktermios *term, struct ktermios *oldterm)
{
        int i;
                                                                                         
        printk(KERN_ERR "termios structure compare\n");

	if (term->c_iflag != oldterm->c_iflag)
        	printk(KERN_ERR "c_iflag: %u %u\n", term->c_iflag, oldterm->c_iflag );


	if (term->c_oflag != oldterm->c_oflag)
        	printk(KERN_ERR "c_oflag: %u %u\n", term->c_oflag, oldterm->c_oflag);

	if (term->c_cflag != oldterm->c_cflag)
        	printk(KERN_ERR "c_cflag: %u %u\n", term->c_cflag, oldterm->c_cflag);

	if (term->c_lflag != oldterm->c_lflag)
        	printk(KERN_ERR "c_lflag: %u %u\n", term->c_lflag, oldterm->c_lflag);

        printk(KERN_ERR "c_cc: ");

        for (i=0; i<19; i++) {
		if (term->c_cc[i] != oldterm->c_cc[i])
                	printk(KERN_ERR "cc(%d): old [%c] new [%c]", i,
			term->c_cc[i], oldterm->c_cc[i]);
	}
                                                                                         
        printk(KERN_ERR "\n");

	if (term->c_line != oldterm->c_line)
        	printk(KERN_ERR "c_line: %c %c\n", term->c_line, oldterm->c_line);

	if (term->c_ispeed != oldterm->c_ispeed)
        	printk(KERN_ERR "c_ispeed: %u %u\n", term->c_ispeed, 
			oldterm->c_ispeed);

	if (term->c_ospeed != oldterm->c_ospeed)
        	printk(KERN_ERR "c_ospeed: %u %u\n", term->c_ospeed, 
			oldterm->c_ospeed);
		
                                                                                         
        printk(KERN_ERR "c_ispeed: %u c_ospeed: %u\n", term->c_ispeed,
                        term->c_ospeed);
                                                                                         
        printk(KERN_ERR "END\n");
}
#endif

/* FIX UP modem control here someday......
*/
static void rs_8xx_set_termios(struct tty_struct *tty, struct ktermios *old_termios)
{
	ser_info_t *info = (ser_info_t *)tty->driver_data;

//	 print_termios(tty->termios, old_termios);

	if (   (tty->termios->c_cflag == old_termios->c_cflag)
	    && (   RELEVANT_IFLAG(tty->termios->c_iflag) 
		== RELEVANT_IFLAG(old_termios->c_iflag)))
	  return;

	change_speed(info);

#ifdef MODEM_CONTROL
	if (PORT_IS_SCC(info->state->smc_scc_num)) {

		/* Handle transition to B0 status */
		if ((old_termios->c_cflag & CBAUD) &&
		    !(tty->termios->c_cflag & CBAUD)) {
			info->MCR &= ~(UART_MCR_DTR|UART_MCR_RTS);
			local_irq_disable();
			serial_out(info, UART_MCR, info->MCR);
			local_irq_enable();
		}
	
		/* Handle transition away from B0 status */
		if (!(old_termios->c_cflag & CBAUD) &&
		    (tty->termios->c_cflag & CBAUD)) {
			info->MCR |= UART_MCR_DTR;
			if (!tty->hw_stopped ||
			    !(tty->termios->c_cflag & CRTSCTS)) {
				info->MCR |= UART_MCR_RTS;
			}
			local_irq_disable();
			serial_out(info, UART_MCR, info->MCR);
			local_irq_enable();
		}
	
		/* Handle turning off CRTSCTS */
		if ((old_termios->c_cflag & CRTSCTS) &&
		    !(tty->termios->c_cflag & CRTSCTS)) {
			tty->hw_stopped = 0;
			rs_8xx_start(tty);
		}
	}
#endif

#if 0
	/*
	 * No need to wake up processes in open wait, since they
	 * sample the CLOCAL flag once, and don't recheck it.
	 * XXX  It's not clear whether the current behavior is correct
	 * or not.  Hence, this may change.....
	 */
	if (!(old_termios->c_cflag & CLOCAL) &&
	    (tty->termios->c_cflag & CLOCAL))
		wake_up_interruptible(&info->open_wait);
#endif
}

/*
 * ------------------------------------------------------------
 * rs_close()
 * 
 * This routine is called when the serial port gets closed.  First, we
 * wait for the last remaining data to be sent.  Then, we unlink its
 * async structure from the interrupt chain if necessary, and we free
 * that IRQ if nothing is left in the chain.
 * ------------------------------------------------------------
 */
static void rs_8xx_close(struct tty_struct *tty, struct file * filp)
{
	ser_info_t *info = (ser_info_t *)tty->driver_data;
	struct serial_state *state;
	unsigned long	flags;
	int		idx;
	volatile smc_t	*smcp;
	volatile scc_t	*sccp;

	if (!info || serial_paranoia_check(info, tty->name, "rs_close"))
		return;

	state = info->state;
	
	local_irq_save(flags);
	
	if (tty_hung_up_p(filp)) {
		//MOD_DEC_USE_COUNT;
		local_irq_restore(flags);
		return;
	}
	
#ifdef SERIAL_DEBUG_OPEN
#ifdef SERIAL_DEBUG_SCC
	if (PORT_IS_SCC(info->state->smc_scc_num))
#endif
	printk("rs_close ttys%d, count = %d\n", info->line, state->count);
#endif
	if ((tty->count == 1) && (state->count != 1)) {
		/*
		 * Uh, oh.  tty->count is 1, which means that the tty
		 * structure will be freed.  state->count should always
		 * be one in these conditions.  If it's greater than
		 * one, we've got real problems, since it means the
		 * serial port won't be shutdown.
		 */
		printk("rs_close: bad serial port count; tty->count is 1, "
		       "state->count is %d\n", state->count);
		state->count = 1;
	}
	if (--state->count < 0) {
		printk("rs_close: bad serial port count for ttys%d: %d\n",
		       info->line, state->count);
		state->count = 0;
	}
	if (state->count) {
		DBG_CNT("before DEC-2");
//		MOD_DEC_USE_COUNT;
		local_irq_restore(flags);
		return;
	}
	info->flags |= ASYNC_CLOSING;
	/*
	 * Now we wait for the transmit buffer to clear; and we notify 
	 * the line discipline to only process XON/XOFF characters.
	 */
	tty->closing = 1;
	if (info->closing_wait != ASYNC_CLOSING_WAIT_NONE)
		tty_wait_until_sent(tty, info->closing_wait);
	/*
	 * At this point we stop accepting input.  To do this, we
	 * disable the receive line status interrupts, and tell the
	 * interrupt driver to stop checking the data ready bit in the
	 * line status register.
	 */
	info->read_status_mask &= ~BD_SC_EMPTY;
	if (info->flags & ASYNC_INITIALIZED) {
		idx = PORT_NUM(info->state->smc_scc_num);
		if (PORT_IS_SCC(info->state->smc_scc_num)) {
			sccp = &cpmp->cp_scc[idx];
#ifdef CONFIG_PRxK
			sccp->scc_sccm &= ~(UART_SCCM_RX | UART_SCCM_BSY) ;
#else
			sccp->scc_sccm &= ~UART_SCCM_RX;
#endif
			sccp->scc_gsmrl &= ~SCC_GSMRL_ENR;
		}
		else {
			smcp = &cpmp->cp_smc[idx];
#ifdef CONFIG_PRxK
			smcp->smc_smcm &= ~(SMCM_RX | SMCM_BSY);
#else
			smcp->smc_smcm &= ~SMCM_RX;
#endif
			smcp->smc_smcmr &= ~SMCMR_REN;
		}
		/*
		 * Before we drop DTR, make sure the UART transmitter
		 * has completely drained; this is especially
		 * important if there is a transmit FIFO!
		 */
		rs_8xx_wait_until_sent(tty, info->timeout);
	}
	shutdown(info);
	if (tty->driver->flush_buffer)
		tty->driver->flush_buffer(tty);
	if (tty->ldisc.flush_buffer)
		tty->ldisc.flush_buffer(tty);
	tty->closing = 0;
	info->event = 0;
	info->tty = 0;
	if (info->blocked_open) {
		if (info->close_delay) {
			current->state = TASK_INTERRUPTIBLE;
			schedule_timeout(info->close_delay);
		}
		wake_up_interruptible(&info->open_wait);
	}
	info->flags &= ~(ASYNC_NORMAL_ACTIVE|
			 ASYNC_CLOSING);
	wake_up_interruptible(&info->close_wait);
#ifdef CONFIG_PRxK
	info->frame_cfg.status = FRAMEMODE_OFF;
	info->frame_cfg.txcount = 0;
	info->frame_cfg.rxcount = 0;
	info->frame_cfg.frame_started = 0;
	info->media = 0;
	info->terminator = 0;
	info->bdtx = 0;
#endif

//	MOD_DEC_USE_COUNT;
	local_irq_restore(flags);
}

/*
 * rs_wait_until_sent() --- wait until the transmitter is empty
 */
static void rs_8xx_wait_until_sent(struct tty_struct *tty, int timeout)
{
	ser_info_t *info = (ser_info_t *)tty->driver_data;
	unsigned long orig_jiffies, char_time;
	/*int lsr;*/
	volatile cbd_t *bdp;
	int bd_num;
	
	if (serial_paranoia_check(info, tty->name, "rs_wait_until_sent"))
		return;

#ifdef maybe
	if (info->state->type == PORT_UNKNOWN)
		return;
#endif

	orig_jiffies = jiffies;
	/*
	 * Set the check interval to be 1/5 of the estimated time to
	 * send a single character, and make it at least 1.  The check
	 * interval should also be less than the timeout.
	 * 
	 * Note: we have to use pretty tight timings here to satisfy
	 * the NIST-PCTS.
	 */
	char_time = 1;
	if (timeout)
		char_time = MIN(char_time, timeout);

	if (PORT_IS_SCC(info->state->smc_scc_num)) {
		if (!timeout || timeout > info->timeout) {
#ifdef SERIAL_DEBUG_RS_WAIT_UNTIL_SENT
			printk("rs_wait_until_sent timeout set from (%d) to (%d)\n", timeout, info->timeout);
#endif
			timeout = info->timeout;
		}
	}
#ifdef SERIAL_DEBUG_RS_WAIT_UNTIL_SENT
#ifdef SERIAL_DEBUG_SCC
	if (PORT_IS_SCC(info->state->smc_scc_num)) {
#endif
	printk("In rs_wait_until_sent(%d) check=%lu...", timeout, char_time);
	printk("jiff=%lu...", jiffies);
#ifdef SERIAL_DEBUG_SCC
	}
#endif
#endif

	/* We go through the loop at least once because we can't tell
	 * exactly when the last character exits the shifter.  There can
	 * be at least two characters waiting to be sent after the buffers
	 * are empty.
	 */
	do {
#ifdef SERIAL_DEBUG_RS_WAIT_UNTIL_SENT
#ifdef SERIAL_DEBUG_SCC
		if (PORT_IS_SCC(info->state->smc_scc_num)) {
#endif
		printk("(jiff=%lu)...", jiffies);
#endif
		current->state = TASK_INTERRUPTIBLE;
/*		current->counter = 0;	 make us low-priority */
		schedule_timeout(char_time);
		if (signal_pending(current))
			break;
		if (timeout && time_after(jiffies, orig_jiffies + timeout))
			break;

		/* The 'tx_cur' is really the next buffer to send.  We
		 * have to back up to the previous BD and wait for it
		 * to go.  This isn't perfect, because all this indicates
		 * is the buffer is available.  There are still characters
		 * in the CPM FIFO.
		 */
#ifdef CONFIG_PRxK
		bd_num = (PORT_IS_SCC(info->state->smc_scc_num) ? TX_SCC_NUM_FIFO : TX_NUM_FIFO);
#else
		bd_num = TX_NUM_FIFO;
#endif
		spin_lock(&info->lock);
		bdp = info->tx_cur;
		spin_unlock(&info->lock);
		if (bdp == info->tx_bd_base)
			bdp += (bd_num-1);
		else
			bdp--;
	} while (bdp->cbd_sc & BD_SC_READY);
	current->state = TASK_RUNNING;
#ifdef SERIAL_DEBUG_RS_WAIT_UNTIL_SENT
#ifdef SERIAL_DEBUG_SCC
	if (PORT_IS_SCC(info->state->smc_scc_num))
#endif
	printk("(jiff=%lu)...done\n", jiffies);
#endif
}

/*
 * rs_hangup() --- called by tty_hangup() when a hangup is signaled.
 */
static void rs_8xx_hangup(struct tty_struct *tty)
{
	ser_info_t *info = (ser_info_t *)tty->driver_data;
	struct serial_state *state = info->state;
	
	if (serial_paranoia_check(info, tty->name, "rs_hangup"))
		return;

	state = info->state;
	
	rs_8xx_flush_buffer(tty);
	shutdown(info);
	info->event = 0;
	state->count = 0;
	info->flags &= ~(ASYNC_NORMAL_ACTIVE);
	info->tty = 0;
	wake_up_interruptible(&info->open_wait);
}

/*
 * ------------------------------------------------------------
 * rs_open() and friends
 * ------------------------------------------------------------
 */
static int block_til_ready(struct tty_struct *tty, struct file * filp,
			   ser_info_t *info)
{
#ifdef DO_THIS_LATER
	DECLARE_WAITQUEUE(wait, current);
#endif
	struct serial_state *state = info->state;
	int		retval;
	int		do_clocal = 0;

	/*
	 * If the device is in the middle of being closed, then block
	 * until it's done, and then try again.
	 */
	if (tty_hung_up_p(filp) ||
	    (info->flags & ASYNC_CLOSING)) {
		if (info->flags & ASYNC_CLOSING)
			interruptible_sleep_on(&info->close_wait);
#ifdef SERIAL_DO_RESTART
		if (info->flags & ASYNC_HUP_NOTIFY)
			return -EAGAIN;
		else
			return -ERESTARTSYS;
#else
		return -EAGAIN;
#endif
	}

	/*
	 * If non-blocking mode is set, or the port is not enabled,
	 * then make the check up front and then exit.
	 * If this is an SMC port, we don't have modem control to wait
	 * for, so just get out here.
	 */
	if ((filp->f_flags & O_NONBLOCK) ||
	    (tty->flags & (1 << TTY_IO_ERROR)) ||
	    !(PORT_IS_SCC(info->state->smc_scc_num))) {
		info->flags |= ASYNC_NORMAL_ACTIVE;
		return 0;
	}
	
    //mp[Sep/2005] BugFix 4723
    if (tty->termios->c_cflag & CLOCAL)
        do_clocal = 1;

	/*
	 * Block waiting for the carrier detect and the line to become
	 * free (i.e., not in use by the callout).  While we are in
	 * this loop, state->count is dropped by one, so that
	 * rs_close() knows when to free things.  We restore it upon
	 * exit, either normal or abnormal.
	 */
	retval = 0;
#ifdef DO_THIS_LATER
	if (PORT_IS_SCC(state->smc_scc_num)) {
		add_wait_queue(&info->open_wait, &wait);
#ifdef SERIAL_DEBUG_OPEN
#ifdef SERIAL_DEBUG_SCC
		if (PORT_IS_SCC(info->state->smc_scc_num))
#endif
		printk("block_til_ready before block: ttys%d, count = %d\n",
	       		state->line, state->count);
#endif
		local_irq_disable();
		if (!tty_hung_up_p(filp)) 
			state->count--;
		local_irq_enable();
		info->blocked_open++;
		while (1) {
			local_irq_disable();
			if((tty->termios->c_cflag & CBAUD)){
				info->MCR = serial_in(info,UART_MCR) |
						(UART_MCR_DTR | UART_MCR_RTS);
				serial_out(info, UART_MCR,info->MCR);
			}
			local_irq_enable();
			__set_current_state(TASK_INTERRUPTIBLE);
			if (tty_hung_up_p(filp) ||
			    !(info->flags & ASYNC_INITIALIZED)) {
#ifdef SERIAL_DO_RESTART
				if (info->flags & ASYNC_HUP_NOTIFY)
					retval = -EAGAIN;
				else
					retval = -ERESTARTSYS;	
#else
				retval = -EAGAIN;
#endif
				break;
			}
			if (!(info->flags & ASYNC_CLOSING) &&
			    (do_clocal || (serial_in(info, UART_MSR) &
					   UART_MSR_DCD)))
				break;
			if (signal_pending(current)) {
				retval = -ERESTARTSYS;
				break;
			}
#ifdef SERIAL_DEBUG_OPEN
#ifdef SERIAL_DEBUG_SCC
			if (PORT_IS_SCC(info->state->smc_scc_num))
#endif
			printk("block_til_ready blocking: ttys%d, count = %d\n",
			       info->line, state->count);
#endif
			schedule();
		}
		current->state = TASK_RUNNING;
		remove_wait_queue(&info->open_wait, &wait);
		if (!tty_hung_up_p(filp))
			state->count++;
		info->blocked_open--;
#ifdef SERIAL_DEBUG_OPEN
#ifdef SERIAL_DEBUG_SCC
		if (PORT_IS_SCC(info->state->smc_scc_num))
#endif
		printk("block_til_ready after blocking: ttys%d, count = %d\n",
		       info->line, state->count);
#endif
	}
#endif /* DO_THIS_LATER */
	if (retval)
		return retval;
	info->flags |= ASYNC_NORMAL_ACTIVE;
	return 0;
}

static int get_async_struct(int line, ser_info_t **ret_info)
{
	struct serial_state *sstate;

	sstate = rs_table + line;
	if (sstate->info) {
		sstate->count++;
		*ret_info = (ser_info_t *)sstate->info;
		return 0;
	}
	else {
		return -ENOMEM;
	}
}


/*
 * This routine is called whenever a serial port is opened.  It
 * enables interrupts for a serial port, linking in its async structure into
 * the IRQ chain.   It also performs the serial-specific
 * initialization for the tty structure.
 */
static int rs_8xx_open(struct tty_struct *tty, struct file * filp)
{
	ser_info_t	*info;
	int 		retval, line;

	line = tty->index;
	if ((line < 0) || (line >= NR_PORTS))
		return -ENODEV;
	retval = get_async_struct(line, &info);
	if (retval) {
		printk(KERN_ERR "retval!\n");
		return retval;
	}
	if (serial_paranoia_check(info, tty->name, "rs_open")) 
		return -ENODEV;

#ifdef SERIAL_DEBUG_OPEN
#ifdef SERIAL_DEBUG_SCC
	if (PORT_IS_SCC(info->state->smc_scc_num))
#endif
	printk("rs_open %d, count = %d\n", info->line,
	       info->state->count);
#endif
	tty->driver_data = info;
	info->tty = tty;

	/*
	 * Start up serial port
	 */
	retval = startup(info);
	if (retval)
		return retval;

//	MOD_INC_USE_COUNT;
	retval = block_til_ready(tty, filp, info);
	if (retval) {
#ifdef SERIAL_DEBUG_OPEN
#ifdef SERIAL_DEBUG_SCC
		if (PORT_IS_SCC(info->state->smc_scc_num))
#endif
		printk("rs_open returning after block_til_ready with %d\n",
		       retval);
#endif
//n		MOD_DEC_USE_COUNT;
		return retval;
	}

#ifdef CONFIG_PRxK
	//[RK]Oct/21/05 - if ttyS1 => return to write routine
	if (PORT_IS_SCC(info->state->smc_scc_num)) {
		tty->driver->write = rs_8xx_write;
	}
#endif

#ifdef SERIAL_DEBUG_OPEN
#ifdef SERIAL_DEBUG_SCC
	if (PORT_IS_SCC(info->state->smc_scc_num))
#endif
	printk("rs_open ttys%d successful...", info->line);
#endif
	return 0;
}

/*
 * /proc fs routines....
 */

static int inline line_info(char *buf, struct serial_state *state)
{
#ifdef MODEM_CONTROL 
	ser_info_t *info = (ser_info_t *) state->info;
	char	stat_buf[30], control, status;
#endif
	int	ret;

	ret = sprintf(buf, "%d: uart:%s port:%X irq:%d",
		      state->line,
		      (PORT_IS_SCC(state->smc_scc_num)) ? "SCC" : "SMC",
		      (unsigned int)(state->port), state->irq);

#ifndef CONFIG_PRxK
	if (!state->port || (state->type == PORT_UNKNOWN)) {
		ret += sprintf(buf+ret, "\n");
		return ret;
	}
#endif

#ifdef MODEM_CONTROL
	if (!info || !PORT_IS_SCC(state->smc_scc_num)) {
		ret += sprintf(buf+ret, "\n");
		return ret;
	}
 
	/*
	 * Figure out the current RS-232 lines
	 */
	local_irq_disable();
	status = serial_in(info, UART_MSR);
	control = info ? info->MCR : serial_in(info, UART_MCR);
	local_irq_enable();
	
	stat_buf[0] = 0;
	stat_buf[1] = 0;
	if (control & UART_MCR_RTS)
		strcat(stat_buf, "|RTS");
	if (status & UART_MSR_CTS)
		strcat(stat_buf, "|CTS");
	if (control & UART_MCR_DTR)
		strcat(stat_buf, "|DTR");
	if (status & UART_MSR_DSR)
		strcat(stat_buf, "|DSR");
	if (status & UART_MSR_DCD)
		strcat(stat_buf, "|CD");
	if (status & UART_MSR_RI)
		strcat(stat_buf, "|RI");

	if (info->tty && info->tty->termios) {
		int i, baud_rate;
		i = info->tty->termios->c_cflag & CBAUD;
		if (i >= (sizeof(baud_table)/sizeof(int)))
			baud_rate = 9600;
		else
			baud_rate = baud_table[i];
		ret += sprintf(buf+ret, " baud:%d", baud_rate);
        }

	ret += sprintf(buf+ret, " tx:%d rx:%d",
		      state->icount.tx, state->icount.rx);

	if (state->icount.frame)
		ret += sprintf(buf+ret, " fe:%d", state->icount.frame);
	
	if (state->icount.parity)
		ret += sprintf(buf+ret, " pe:%d", state->icount.parity);
	
	if (state->icount.brk)
		ret += sprintf(buf+ret, " brk:%d", state->icount.brk);	

	if (state->icount.overrun)
		ret += sprintf(buf+ret, " oe:%d", state->icount.overrun);

	/*
	 * Last thing is the RS-232 status lines
	 */
	ret += sprintf(buf+ret, " %s\n", stat_buf+1);
#endif
	return ret;
}

int rs_8xx_read_proc(char *page, char **start, off_t off, int count,
		 int *eof, void *data)
{
	int i, len = 0;
	off_t	begin = 0;

	len += sprintf(page, "serinfo:1.0 driver:%s\n", serial_version);
	for (i = 0; i < NR_PORTS && len < 4000; i++) {
		len += line_info(page + len, &rs_table[i]);
		if (len+begin > off+count)
			goto done;
		if (len+begin < off) {
			begin += len;
			len = 0;
		}
	}
	*eof = 1;
done:
	if (off >= len+begin)
		return 0;
	*start = page + (begin-off);
	return ((count < begin+len-off) ? count : begin+len-off);
}

/*
 * ---------------------------------------------------------------------
 * rs_init() and friends
 *
 * rs_init() is called at boot-time to initialize the serial driver.
 * ---------------------------------------------------------------------
 */

/*
 * This routine prints out the appropriate serial driver version
 * number, and identifies which options were configured into this
 * driver.
 */
static _INLINE_ void show_serial_version(void)
{
 	printk(KERN_INFO "%s version %s\n", serial_name, serial_version);
}


/*
 * The serial console driver used during boot.  Note that these names
 * clash with those found in "serial.c", so we currently can't support
 * the 16xxx uarts and these at the same time.  I will fix this to become
 * an indirect function call from tty_io.c (or something).
 */

#ifdef CONFIG_SERIAL_CONSOLE

/* I need this just so I can store the virtual addresses and have
 * common functions for the early console printing.
 */
static ser_info_t consinfo;

/*
 * Print a string to the serial port trying not to disturb any possible
 * real use of the port...
 */
static void my_console_write(int idx, const char *s,
				unsigned count)
{
	struct		serial_state	*ser;
	ser_info_t			*info;
	unsigned			i;
	volatile	cbd_t		*bdp, *bdbase;
	volatile	smc_uart_t	*up;
	volatile	u_char		*cp;

	ser = rs_table + idx;

	/* If the port has been initialized for general use, we have
	 * to use the buffer descriptors allocated there.  Otherwise,
	 * we simply use the single buffer allocated.
	 */
	if ((info = (ser_info_t *)ser->info) != NULL) {
		bdp = info->tx_cur;
		bdbase = info->tx_bd_base;
	}
	else {
		/* Pointer to UART in parameter ram.
		*/
		up = (smc_uart_t *)&cpmp->cp_dparam[ser->port];

		/* Get the address of the host memory buffer.
		 */
		bdp = bdbase = (cbd_t *)&cpmp->cp_dpmem[up->smc_tbase];

		info = &consinfo;
	}

	/*
	 * We need to gracefully shut down the transmitter, disable
	 * interrupts, then send our bytes out.
	 */

	/*
	 * Now, do each character.  This is not as bad as it looks
	 * since this is a holding FIFO and not a transmitting FIFO.
	 * We could add the complexity of filling the entire transmit
	 * buffer, but we would just wait longer between accesses......
	 */
	for (i = 0; i < count; i++, s++) {
		/* Wait for transmitter fifo to empty.
		 * Ready indicates output is ready, and xmt is doing
		 * that, not that it is ready for us to send.
		 */
		while (bdp->cbd_sc & BD_SC_READY);

		/* Send the character out.
		 * If the buffer address is in the CPM DPRAM, don't
		 * convert it.
		 */
		if ((uint)(bdp->cbd_bufaddr) > (uint)IMAP_ADDR)
			cp = (u_char *)(bdp->cbd_bufaddr);
		else
			cp = info->tx_va_base + ((bdp - info->tx_bd_base) * TX_BUF_SIZE);
		*cp = *s;
		
		bdp->cbd_datlen = 1;
#ifdef CONFIG_PRxK
		if (IS_BOARD_ACS1_KVM_TS1H && (idx))  // scc
			bdp->cbd_sc |= BD_SC_CR;
#endif
		bdp->cbd_sc |= BD_SC_READY;

		if (bdp->cbd_sc & BD_SC_WRAP)
			bdp = bdbase;
		else
			bdp++;

		/* if a LF, also do CR... */
		if (*s == 10) {
			while (bdp->cbd_sc & BD_SC_READY);
			cp = (u_char *) info->tx_va_base + ((bdp - info->tx_bd_base) * TX_BUF_SIZE);
			*cp = 13;
			bdp->cbd_datlen = 1;
#ifdef CONFIG_PRxK
			if (IS_BOARD_ACS1_KVM_TS1H && (idx))  // scc
				bdp->cbd_sc |= BD_SC_CR;
#endif
			bdp->cbd_sc |= BD_SC_READY;

			if (bdp->cbd_sc & BD_SC_WRAP) {
				bdp = bdbase;
			}
			else {
				bdp++;
			}
		}
	}

	/*
	 * Finally, Wait for transmitter & holding register to empty
	 *  and restore the IER
	 */
	while (bdp->cbd_sc & BD_SC_READY);

	if (info)
		info->tx_cur = (cbd_t *)bdp;
}

static void serial_console_write(struct console *c, const char *s,
				unsigned count)
{
#ifdef CONFIG_KGDB
	/* Try to let stub handle output. Returns true if it did. */
	if (c->index == KGDB_SER_IDX)
		if (kgdb_output_string(s, count))
			return;
#endif
	my_console_write(c->index, s, count);
}

#ifdef CONFIG_XMON
int
xmon_8xx_write(const char *s, unsigned count)
{
	my_console_write(KGDB_SER_IDX, s, count);
	return(count);
}
#endif

#ifdef CONFIG_KGDB
void
putDebugChar(char ch)
{
	my_console_write(KGDB_SER_IDX, &ch, 1);
}
#endif

#if defined(CONFIG_XMON) || defined(CONFIG_KGDB)
/*
 * Receive character from the serial port.  This only works well
 * before the port is initialized for real use.
 */
static int my_console_wait_key(int idx, int xmon, char *obuf)
{
	struct serial_state		*ser;
	u_char				c, *cp;
	ser_info_t			*info;
	volatile	cbd_t		*bdp;
	volatile	smc_uart_t	*up;
	int				i;

	ser = rs_table + idx;

	/* Pointer to UART in parameter ram.
	*/
	up = (smc_uart_t *)&cpmp->cp_dparam[ser->port];

	/* Get the address of the host memory buffer.
	 * If the port has been initialized for general use, we must
	 * use information from the port structure.
	 */
	if ((info = (ser_info_t *)ser->info)) {
		bdp = info->rx_cur;
	} else {
		bdp = (cbd_t *)&cpmp->cp_dpmem[up->smc_rbase];
		info = &consinfo;
	}

	/*
	 * We need to gracefully shut down the receiver, disable
	 * interrupts, then read the input.
	 * XMON just wants a poll.  If no character, return -1, else
	 * return the character.
	 */
	if (!xmon) {
		while (bdp->cbd_sc & BD_SC_EMPTY);
	}
	else {
		if (bdp->cbd_sc & BD_SC_EMPTY)
			return -1;
	}

	/* If the buffer address is in the CPM DPRAM, don't
	 * convert it.
	 */
	if ((uint)(bdp->cbd_bufaddr) > (uint)IMAP_ADDR)
		cp = (u_char *)(bdp->cbd_bufaddr);
	else
		cp = (u_char *) info->rx_va_base + ((bdp - info->rx_bd_base) * RX_BUF_SIZE);

	if (obuf) {
		i = c = bdp->cbd_datlen;
		while (i-- > 0)
			*obuf++ = *cp++;
	}
	else {
		c = *cp;
	}
	bdp->cbd_sc |= BD_SC_EMPTY;

	if (info) {
		if (bdp->cbd_sc & BD_SC_WRAP) {
			bdp = info->rx_bd_base;
		}
		else {
			bdp++;
		}
		info->rx_cur = (cbd_t *)bdp;
	}

	return((int)c);
}
#endif

#ifdef CONFIG_XMON
int
xmon_8xx_read_poll(void)
{
	return(my_console_wait_key(KGDB_SER_IDX, 1, NULL));
}

int
xmon_8xx_read_char(void)
{
	return(my_console_wait_key(KGDB_SER_IDX, 0, NULL));
}
#endif

#ifdef CONFIG_KGDB
static char kgdb_buf[RX_BUF_SIZE], *kgdp;
static int kgdb_chars;

char
getDebugChar(void)
{
	if (kgdb_chars <= 0) {
		kgdb_chars = my_console_wait_key(KGDB_SER_IDX, 0, kgdb_buf);
		kgdp = kgdb_buf;
	}
	kgdb_chars--;

	return(*kgdp++);
}

void kgdb_interruptible(int yes)
{
	volatile smc_t	*smcp;
	volatile scc_t	*sccp;
	struct serial_state *ser;

	ser = rs_table + KGDB_SER_IDX;

	if (PORT_IS_SCC(ser->smc_scc_num)) {
		sccp = &cpmp->cp_scc[PORT_NUM(ser->smc_scc_num)];

		if (yes == 1)
#ifdef CONFIG_PRxK
			sccp->scc_sccm |= UART_SCCM_RX | UART_SCCM_BSY;
#else
			sccp->scc_sccm |= UART_SCCM_RX;
#endif
		else
#ifdef CONFIG_PRxK
			sccp->scc_sccm &= ~(UART_SCCM_RX | UART_SCCM_BSY);
#else
			sccp->scc_sccm &= ~UART_SCCM_RX;
#endif
	}
	else {
		smcp = &cpmp->cp_smc[PORT_NUM(ser->smc_scc_num)];

		if (yes == 1)
#ifdef CONFIG_PRxK
			smcp->smc_sccm |= SMCM_RX | SMCM_BSY;
#else
			smcp->smc_smcm |= SMCM_RX;
#endif
		else
#ifdef CONFIG_PRxK
			smcp->smc_smcm &= ~(SMCM_RX|SMCM_BSY);
#else
			smcp->smc_smcm &= ~SMCM_RX;
#endif
	}
}

void kgdb_map_scc(void)
{
	struct		serial_state *ser;
	uint		mem_addr;
	volatile	cbd_t		*bdp;
	volatile	smc_uart_t	*up;
	volatile	scc_uart_t	*scup;

	cpmp = (cpm8xx_t *)&(((immap_t *)IMAP_ADDR)->im_cpm);

	/* To avoid data cache CPM DMA coherency problems, allocate a
	 * buffer in the CPM DPRAM.  This will work until the CPM and
	 * serial ports are initialized.  At that time a memory buffer
	 * will be allocated.
	 * The port is already initialized from the boot procedure, all
	 * we do here is give it a different buffer and make it a FIFO.
	 */

	ser = rs_table + KGDB_SER_IDX;

	/* Allocate space for an input FIFO, plus a few bytes for output.
	 * Allocate bytes to maintain word alignment.
	 */
	mem_addr = (uint)(&cpmp->cp_dpmem[KGDB_DPRAM_OFFSET]);

	if (PORT_IS_SCC(ser->smc_scc_num)) {
		scup = (scc_uart_t *)&cpmp->cp_dparam[ser->port];

		/* Set the physical address of the host memory buffers in
		 * the buffer descriptors.
		 */
		bdp = (cbd_t *)&cpmp->cp_dpmem[scup->scc_genscc.scc_rbase];
		bdp->cbd_bufaddr = mem_addr;

		bdp = (cbd_t *)&cpmp->cp_dpmem[scup->scc_genscc.scc_tbase];
		bdp->cbd_bufaddr = mem_addr+RX_SCC_BUF_SIZE;

		scup->scc_genscc.scc_mrblr = RX_SCC_BUF_SIZE;
		scup->scc_maxidl = CONFIG_UART_MAXIDL_SCC1;
	}
	else {
		up = (smc_uart_t *)&cpmp->cp_dparam[ser->port];

		/* Set the physical address of the host memory buffers in
		 * the buffer descriptors.
		 */
		bdp = (cbd_t *)&cpmp->cp_dpmem[up->smc_rbase];
		bdp->cbd_bufaddr = mem_addr;

		bdp = (cbd_t *)&cpmp->cp_dpmem[up->smc_tbase];
		bdp->cbd_bufaddr = mem_addr+RX_BUF_SIZE;

		up->smc_mrblr = RX_BUF_SIZE;
		up->smc_maxidl = RX_BUF_SIZE;
	}
}
#endif

static struct tty_driver *serial_console_device(struct console *c, int *index)
{
       *index = c->index;
       return serial_driver;
}

/*
 *	Register console.
 */
static int __init console_8xx_init(void)
{
	register_console(&sercons);
	return 0;
}
console_initcall(console_8xx_init);

#endif

/* Index in baud rate table of the default console baud rate.
*/
static	int	baud_idx;

static struct tty_operations rs_8xx_ops = {
       .open = rs_8xx_open,
       .close = rs_8xx_close,
       .write = rs_8xx_write,
       .put_char = rs_8xx_put_char,
       .write_room = rs_8xx_write_room,
       .chars_in_buffer = rs_8xx_chars_in_buffer,
       .flush_buffer = rs_8xx_flush_buffer,
       .ioctl = rs_8xx_ioctl,
       .throttle = rs_8xx_throttle,
       .unthrottle = rs_8xx_unthrottle,
       .send_xchar = rs_8xx_send_xchar,
       .set_termios = rs_8xx_set_termios,
       .stop = rs_8xx_stop,
       .start = rs_8xx_start,
       .hangup = rs_8xx_hangup,
       .wait_until_sent = rs_8xx_wait_until_sent,
       .read_proc = rs_8xx_read_proc,
       .tiocmget = rs_tiocmget,
       .tiocmset = rs_tiocmset,
};


/*
 * The serial driver boot-time initialization code!
 */
int __init rs_8xx_init(void)
{
	struct serial_state * state;
	ser_info_t	*info;
	void		*mem_addr;
	uint		dp_addr, iobits;
	int		i, j, idx;
	ushort		chan;
	volatile	cbd_t		*bdp;
	volatile	cpm8xx_t	*cp;
	volatile	smc_t		*sp  = NULL;
	volatile	smc_uart_t	*up  = NULL;
	volatile	scc_t		*scp = NULL;
	volatile	scc_uart_t	*sup = NULL;
	volatile	immap_t		*immap;
	dma_addr_t	physaddr;

	int	numb;

	serial_driver = alloc_tty_driver(NR_PORTS);
	if (!serial_driver)
		return -ENOMEM;

	/* ACS1 board */

	if (IS_BOARD_ACS1 || IS_BOARD_TS1H) {
		dtr1_pin = (1 << (31 - CONFIG_DTR1_PIN_ACS1));
	} else {
		if (IS_BOARD_KVM) {
			dtr1_pin = (1 << (31 - CONFIG_DTR1_PIN_KVM));
		}
	}

	show_serial_version();

	serial_driver->owner = THIS_MODULE;
	serial_driver->driver_name = "serial";
	serial_driver->name = "ttyS";
	serial_driver->major = TTY_MAJOR;
	serial_driver->minor_start = 64;
	serial_driver->type = TTY_DRIVER_TYPE_SERIAL;
	serial_driver->subtype = SERIAL_TYPE_NORMAL;
	serial_driver->init_termios = tty_std_termios;
	serial_driver->init_termios.c_cflag =
		baud_idx | CS8 | CREAD | HUPCL | CLOCAL;

	serial_driver->flags = TTY_DRIVER_REAL_RAW;
	tty_set_operations(serial_driver, &rs_8xx_ops);

	if (tty_register_driver(serial_driver))
		panic("Couldn't register serial driver\n");
	
	cp = cpmp;	/* Get pointer to Communication Processor */
	immap = (immap_t *)IMAP_ADDR;	/* and to internal registers */


	/* Configure SCC2, SCC3, and SCC4 instead of port A parallel I/O.
	 */
#ifdef CONFIG_USE_SCC_IO
#ifndef CONFIG_MBX
	/* The "standard" configuration through the 860.
	*/
#ifndef CONFIG_TSxK
	immap->im_ioport.iop_papar |= 0x00fc;
	immap->im_ioport.iop_padir &= ~0x00fc;
	immap->im_ioport.iop_paodr &= ~0x00fc;
#endif
#else
	/* On the MBX, SCC3 is through Port D.
	*/
	immap->im_ioport.iop_papar |= 0x000c;	/* SCC2 on port A */
	immap->im_ioport.iop_padir &= ~0x000c;
	immap->im_ioport.iop_paodr &= ~0x000c;

	immap->im_ioport.iop_pdpar |= 0x0030;	/* SCC3 on port D */
#endif

	/* Since we don't yet do modem control, connect the port C pins
	 * as general purpose I/O.  This will assert CTS and CD for the
	 * SCC ports.
	 */
#ifndef CONFIG_TSxK
	immap->im_ioport.iop_pcdir |= 0x03c6;
	immap->im_ioport.iop_pcpar &= ~0x03c6;
#endif

#ifdef CONFIG_PP04
	/* Frequentis PP04 forced to RS-232 until we know better.
	 * Port C 12 and 13 low enables RS-232 on SCC3 and SCC4.
	 */
	immap->im_ioport.iop_pcdir |= 0x000c;
	immap->im_ioport.iop_pcpar &= ~0x000c;
	immap->im_ioport.iop_pcdat &= ~0x000c;

	/* This enables the TX driver.
	*/
	cp->cp_pbpar &= ~0x6000;
	cp->cp_pbdat &= ~0x6000;
#endif
#endif

       //az - ACS1's scc
       // RK??????
       immap->im_ioport.iop_papar |=  0x03;
       immap->im_ioport.iop_padir &= ~0x03;
       immap->im_ioport.iop_paodr &= ~0x03;

       immap->im_ioport.iop_pcdir |=  0x30;
       immap->im_ioport.iop_pcpar &= ~0x30;

       immap->im_ioport.iop_pcdir &= ~0x08;
       immap->im_ioport.iop_pcpar &= ~0x08;

       immap->im_cpm.cp_pbpar &= ~(1 << (31 - CONFIG_RTS1_PIN));
       immap->im_cpm.cp_pbdir |= (1 << (31 - CONFIG_RTS1_PIN));

       immap->im_ioport.iop_pcpar &= ~0x30;
       immap->im_ioport.iop_pcso &= ~0x30; // mp[set2003] It was |= 0x30...
       immap->im_ioport.iop_pcdir &= ~0x30;

       PORT_DTR1_PAR &= ~dtr1_pin;     /* DTR1 */
       PORT_DTR1_DAT |=  dtr1_pin;
       PORT_DTR1_DIR |=  dtr1_pin;

       immap->im_ioport.iop_pcdir |= 0x0200;  // makes PC6 un output pin
       // az - ACS1 end


	for (i = 0, state = rs_table; i < NR_PORTS; i++,state++) {
		state->magic = SSTATE_MAGIC;
		state->line = i;
		state->type = PORT_UNKNOWN;
		state->custom_divisor = 0;
		state->close_delay = 5*HZ/10;
		state->closing_wait = 30*HZ;
		state->icount.cts = state->icount.dsr = 
			state->icount.rng = state->icount.dcd = 0;
		state->icount.rx = state->icount.tx = 0;
		state->icount.frame = state->icount.parity = 0;
		state->icount.overrun = state->icount.brk = 0;
		printk(KERN_INFO "ttyS%d on %s%d at 0x%04x, BRG%d\n",
		       i,
		       (PORT_IS_SCC(state->smc_scc_num)) ? "SCC" : "SMC",
		       PORT_NUM(state->smc_scc_num) + 1,
		       (unsigned int)(state->port),
		       state->line + 1);

#ifdef CONFIG_SERIAL_CONSOLE
		/* If we just printed the message on the console port, and
		 * we are about to initialize it for general use, we have
		 * to wait a couple of character times for the CR/NL to
		 * make it out of the transmit buffer.
		 */
		if (i == CONFIG_SERIAL_CONSOLE_PORT)
			mdelay(2);
#endif
		info = kmalloc(sizeof(ser_info_t), GFP_KERNEL);
		if (info) {
			__clear_user(info,sizeof(ser_info_t));
			info->magic = SERIAL_MAGIC;
                        info->flags = state->flags;
                        INIT_WORK(&info->work, do_softint);
                        INIT_WORK(&info->hangup, do_serial_hangup);
                        init_waitqueue_head(&info->open_wait);
                        init_waitqueue_head(&info->close_wait);
			info->line = i;
			info->state = state;
			state->info = (struct async_struct *) info;

#ifdef CONFIG_PRxK
			init_waitqueue_head(&info->delta_msr_wait);
			spin_lock_init(&info->lock);
			//az - ACS1's framemode init
			info->frame_cfg.status = FRAMEMODE_OFF;
			info->frame_cfg.rx_stat.success = 0;
			info->frame_cfg.rx_stat.error = 0;
			info->frame_cfg.tx_stat.success = 0;
			info->frame_cfg.tx_stat.error = 0;

			info->hw_stopped = 0;
			info->bdtx = 0;
			info->media = 0;
#endif
			/* We need to allocate a transmit and receive buffer
			 * descriptors from dual port ram, and a character
			 * buffer area from host mem.
			 */
#ifdef CONFIG_PRxK
			numb = PORT_IS_SCC(state->smc_scc_num) ? RX_SCC_NUM_FIFO : RX_NUM_FIFO;
			dp_addr = m8xx_cpm_dpalloc(sizeof(cbd_t) * numb);
#else
			dp_addr = m8xx_cpm_dpalloc(sizeof(cbd_t) * RX_NUM_FIFO);
#endif

			/* Allocate space for FIFOs in the host memory.
			*/
#ifdef CONFIG_PRxK
			j = PORT_IS_SCC(state->smc_scc_num) ? RX_SCC_BUF_SIZE : RX_BUF_SIZE;
			mem_addr = dma_alloc_coherent(NULL, (j * numb), &physaddr, GFP_KERNEL);
#else
			mem_addr = dma_alloc_coherent(NULL, RX_NUM_FIFO * RX_BUF_SIZE, &physaddr, GFP_KERNEL);
#endif
			info->rx_va_base = (unsigned char *)mem_addr;

			/* Set the physical address of the host memory
			 * buffers in the buffer descriptors, and the
			 * virtual address for us to work with.
			 */
			bdp = (cbd_t *)&cp->cp_dpmem[dp_addr];
			info->rx_cur = info->rx_bd_base = (cbd_t *)bdp;
#ifdef CONFIG_PRxK
			for (j=0; j < (numb-1); j++)
#else
			for (j=0; j<(RX_NUM_FIFO-1); j++) 
#endif
			{
				bdp->cbd_bufaddr = physaddr;
				bdp->cbd_sc = BD_SC_EMPTY | BD_SC_INTRPT;
#ifdef CONFIG_PRxK
				physaddr += (PORT_IS_SCC(state->smc_scc_num) ? RX_SCC_BUF_SIZE : RX_BUF_SIZE);
#else
				physaddr += RX_BUF_SIZE;
#endif
				bdp++;
			}
			bdp->cbd_bufaddr = physaddr;
			bdp->cbd_sc = BD_SC_WRAP | BD_SC_EMPTY | BD_SC_INTRPT;


			idx = PORT_NUM(state->smc_scc_num);
			if (PORT_IS_SCC(state->smc_scc_num)) {
				scp = &cp->cp_scc[idx];
				sup = (scc_uart_t *)&cp->cp_dparam[state->port];
				sup->scc_genscc.scc_rbase = dp_addr;
			}
			else {
				sp = &cp->cp_smc[idx];
				up = (smc_uart_t *)&cp->cp_dparam[state->port];
				up->smc_rbase = dp_addr;
			}
#ifdef CONFIG_PRxK
			numb = PORT_IS_SCC(state->smc_scc_num) ? TX_SCC_NUM_FIFO : TX_NUM_FIFO;
			dp_addr = m8xx_cpm_dpalloc(sizeof(cbd_t) * numb);
#else
			dp_addr = m8xx_cpm_dpalloc(sizeof(cbd_t) * TX_NUM_FIFO);
#endif

			/* Allocate space for FIFOs in the host memory.
			*/
#ifdef CONFIG_PRxK
			j = PORT_IS_SCC(state->smc_scc_num) ? TX_SCC_BUF_SIZE : TX_BUF_SIZE;
			mem_addr = dma_alloc_coherent(NULL, j * numb, &physaddr, GFP_KERNEL);
#else
			mem_addr = dma_alloc_coherent(NULL, TX_NUM_FIFO * TX_BUF_SIZE, &physaddr, GFP_KERNEL);
#endif 
			info->tx_va_base = (unsigned char *)mem_addr;

			/* Set the physical address of the host memory
			 * buffers in the buffer descriptors, and the
			 * virtual address for us to work with.
			 */
			bdp = (cbd_t *)&cp->cp_dpmem[dp_addr];
			info->tx_cur = info->tx_bd_base = (cbd_t *)bdp;

#ifdef CONFIG_PRxK
			for (j=0; j < (numb-1); j++)
#else
			for (j=0; j<(TX_NUM_FIFO-1); j++) 
#endif
			{
				bdp->cbd_bufaddr = physaddr;
				bdp->cbd_sc = BD_SC_INTRPT;
#ifdef CONFIG_PRxK
				physaddr += (PORT_IS_SCC(state->smc_scc_num) ? TX_SCC_BUF_SIZE : TX_BUF_SIZE);
#else
				physaddr += TX_BUF_SIZE;
#endif
				bdp++;
			}
			bdp->cbd_bufaddr = physaddr;
			bdp->cbd_sc = (BD_SC_WRAP | BD_SC_INTRPT);

			if (PORT_IS_SCC(state->smc_scc_num)) {
				sup->scc_genscc.scc_tbase = dp_addr;

				/* Set up the uart parameters in the
				 * parameter ram.
				 */
				sup->scc_genscc.scc_rfcr = SMC_EB;
				sup->scc_genscc.scc_tfcr = SMC_EB;

				/* Set this to 1 for now, so we get single
				 * character interrupts.  Using idle charater
				 * time requires some additional tuning.
				 */
				sup->scc_genscc.scc_mrblr = 1;
				sup->scc_maxidl = 0;
				sup->scc_brkcr = 1;
				sup->scc_parec = 0;
				sup->scc_frmec = 0;
				sup->scc_nosec = 0;
				sup->scc_brkec = 0;
				sup->scc_uaddr1 = 0;
				sup->scc_uaddr2 = 0;
				sup->scc_toseq = 0;
				sup->scc_char1 = 0x8000;
				sup->scc_char2 = 0x8000;
				sup->scc_char3 = 0x8000;
				sup->scc_char4 = 0x8000;
				sup->scc_char5 = 0x8000;
				sup->scc_char6 = 0x8000;
				sup->scc_char7 = 0x8000;
				sup->scc_char8 = 0x8000;
				sup->scc_rccm = 0xc0ff;

				/* Send the CPM an initialize command.
				*/
				chan = scc_chan_map[idx];

				cp->cp_cpcr = mk_cr_cmd(chan,
						CPM_CR_INIT_TRX) | CPM_CR_FLG;
				while (cp->cp_cpcr & CPM_CR_FLG);

				/* Set UART mode, 8 bit, no parity, one stop.
				 * Enable receive and transmit.
				 */
				scp->scc_gsmrh = 0;
#ifdef CONFIG_PRxK
				if (IS_BOARD_ACS1_KVM_TS1H)
					scp->scc_gsmrh = SCC_GSMRH_RTSM;
#endif
				scp->scc_gsmrl = 
					(SCC_GSMRL_MODE_UART | SCC_GSMRL_TDCR_16 | SCC_GSMRL_RDCR_16);

				/* Disable all interrupts and clear all pending
				 * events.
				 */
				scp->scc_sccm = 0;
				scp->scc_scce = 0xffff;
				scp->scc_dsr = 0x7e7e;
				scp->scc_psmr = 0x3000;

				/* If the port is the console, enable Rx and Tx.
				*/
#ifdef CONFIG_SERIAL_CONSOLE
				if (IS_BOARD_ACS1_KVM_TS1H &&
				    (i == CONFIG_SERIAL_CONSOLE_PORT))
					scp->scc_gsmrl |= (SCC_GSMRL_ENR | SCC_GSMRL_ENT);
#endif
			}
			else {
				/* Configure SMCs Tx/Rx instead of port B
				 * parallel I/O.  On 823/850 these are on
				 * port A for SMC2.
				 */
#ifndef CONFIG_ALTSMC2
#ifndef CONFIG_TSxK
				iobits = 0xc0 << (idx * 4);
				cp->cp_pbpar |= iobits;
				cp->cp_pbdir &= ~iobits;
				cp->cp_pbodr &= ~iobits;
#endif
#else
				iobits = 0xc0;
				if (idx == 0) {
					/* SMC1 on Port B, like all 8xx.
					*/
					cp->cp_pbpar |= iobits;
					cp->cp_pbdir &= ~iobits;
					cp->cp_pbodr &= ~iobits;
				}
				else {
					/* SMC2 is on Port A.
					*/
					immap->im_ioport.iop_papar |= iobits;
					immap->im_ioport.iop_padir &= ~iobits;
					immap->im_ioport.iop_paodr &= ~iobits;
				}
#endif /* CONFIG_ALTSMC2 */

				up->smc_tbase = dp_addr;

				/* Set up the uart parameters in the
				 * parameter ram.
				 */
				up->smc_rfcr = SMC_EB;
				up->smc_tfcr = SMC_EB;

				/* Set this to 1 for now, so we get single
				 * character interrupts.  Using idle charater
				 * time requires some additional tuning.
				 */
				up->smc_mrblr = 1;
				up->smc_maxidl = 0;
				up->smc_brkcr = 1;

				/* Send the CPM an initialize command.
				*/
				chan = smc_chan_map[idx];

				cp->cp_cpcr = mk_cr_cmd(chan,
						CPM_CR_INIT_TRX) | CPM_CR_FLG;
				while (cp->cp_cpcr & CPM_CR_FLG);

				/* Set UART mode, 8 bit, no parity, one stop.
				 * Enable receive and transmit.
				 */
				sp->smc_smcmr = smcr_mk_clen(9) | SMCMR_SM_UART;

				/* Disable all interrupts and clear all pending
				 * events.
				 */
				sp->smc_smcm = 0;
				sp->smc_smce = 0xff;

				/* If the port is the console, enable Rx and Tx.
				*/
#ifdef CONFIG_SERIAL_CONSOLE
				if (i == CONFIG_SERIAL_CONSOLE_PORT)
					sp->smc_smcmr |= SMCMR_REN | SMCMR_TEN;
#endif
			}

			/* Install interrupt handler.
			*/
			cpm_install_handler(state->irq, rs_8xx_interrupt, info);

			/** \todo FIX THIS or THE TABLE! */
			if(state->smc_scc_num & NUM_IS_SCC){
	                        cpm_install_handler(CPMVEC_PIO_PC10, rs_8xx_dcd_interrupt, info);
        	                cpm_install_handler(CPMVEC_PIO_PC11, rs_8xx_cts_interrupt, info);
			}

			/* Set up the baud rate generator.
			*/
#ifdef CONFIG_NL1K
			m8xx_cpm_setbrg(i + 1, baud_table[baud_idx]);
			connect_brg(state, i + 1);
#else
			m8xx_cpm_setbrg(i, baud_table[baud_idx]);
			connect_brg(state, i);
#endif

		}
	}

	return 0;
}

module_init(rs_8xx_init);


/* This must always be called before the rs_8xx_init() function, otherwise
 * it blows away the port control information.
*/
static int __init serial_console_setup(struct console *co, char *options)
{
	struct		serial_state *ser;
	unsigned char	*mem_addr;
	uint		dp_addr, bidx, idx;
	ushort		chan;
	volatile	cbd_t		*bdp;
	volatile	cpm8xx_t	*cp;
	volatile	smc_t		*sp  = NULL;
	volatile	scc_t		*scp = NULL;
	volatile	smc_uart_t	*up  = NULL;
	volatile	scc_uart_t	*sup = NULL;
	bd_t				*bd;

	bd = (bd_t *)__res;

	for (bidx = 0; bidx < (sizeof(baud_table) / sizeof(int)); bidx++)
		if (bd->bi_baudrate == baud_table[bidx])
			break;
	/* make sure we have a useful value */
	if (bidx == (sizeof(baud_table) / sizeof(int)))
		bidx = 13;	/* B9600 */

	co->cflag = CREAD|CLOCAL|bidx|CS8;
	baud_idx = bidx;

	ser = rs_table + co->index;

	cp = cpmp;	/* Get pointer to Communication Processor */

	idx = PORT_NUM(ser->smc_scc_num);
	if (PORT_IS_SCC(ser->smc_scc_num)) {
		scp = &cp->cp_scc[idx];
		sup = (scc_uart_t *)&cp->cp_dparam[ser->port];
	}
	else {
		sp = &cp->cp_smc[idx];
		up = (smc_uart_t *)&cpmp->cp_dparam[ser->port];
	}

	/* When we get here, the CPM has been reset, so we need
	 * to configure the port.
	 * We need to allocate a transmit and receive buffer descriptor
	 * from dual port ram, and a character buffer area from host mem.
	 */

	/* Allocate space for two 2 byte FIFOs in the dpram memory:
	 * the vm allocator is not up yet.
	 */
	dp_addr = m8xx_cpm_dpalloc(8);

	mem_addr = (unsigned char *) &cp->cp_dpmem[dp_addr]; 

	/* Allocate space for two buffer descriptors in the DP ram.
	*/
	dp_addr = m8xx_cpm_dpalloc(sizeof(cbd_t) * 2);

	/* Set the physical address of the host memory buffers in
	 * the buffer descriptors.
	 */
	bdp = (cbd_t *)&cp->cp_dpmem[dp_addr];
	bdp->cbd_bufaddr = (unsigned long) mem_addr;
	(bdp+1)->cbd_bufaddr = (unsigned long) (mem_addr + 4);

	consinfo.rx_va_base = mem_addr;
	consinfo.rx_bd_base = (cbd_t *) bdp;
	consinfo.tx_va_base = mem_addr + 4;
	consinfo.tx_bd_base = (cbd_t *) (bdp + 1);

	/* For the receive, set empty and wrap.
	 * For transmit, set wrap.
	 */
	bdp->cbd_sc = BD_SC_EMPTY | BD_SC_WRAP;
	(bdp+1)->cbd_sc = BD_SC_WRAP;

	/* Set up the uart parameters in the parameter ram.
	*/
	if (PORT_IS_SCC(ser->smc_scc_num)) {

		sup->scc_genscc.scc_rbase = dp_addr;
		sup->scc_genscc.scc_tbase = dp_addr + sizeof(cbd_t);

		/* Set up the uart parameters in the
		 * parameter ram.
		 */
		sup->scc_genscc.scc_rfcr = SMC_EB;
		sup->scc_genscc.scc_tfcr = SMC_EB;

		/* Set this to 1 for now, so we get single
		 * character interrupts.  Using idle charater
		 * time requires some additional tuning.
		 */
		sup->scc_genscc.scc_mrblr = 1;
		sup->scc_maxidl = 0;
		sup->scc_brkcr = 1;
		sup->scc_parec = 0;
		sup->scc_frmec = 0;
		sup->scc_nosec = 0;
		sup->scc_brkec = 0;
		sup->scc_uaddr1 = 0;
		sup->scc_uaddr2 = 0;
		sup->scc_toseq = 0;
		sup->scc_char1 = 0x8000;
		sup->scc_char2 = 0x8000;
		sup->scc_char3 = 0x8000;
		sup->scc_char4 = 0x8000;
		sup->scc_char5 = 0x8000;
		sup->scc_char6 = 0x8000;
		sup->scc_char7 = 0x8000;
		sup->scc_char8 = 0x8000;
		sup->scc_rccm = 0xc0ff;

		/* Send the CPM an initialize command.
		*/
		chan = scc_chan_map[idx];

		cp->cp_cpcr = mk_cr_cmd(chan, CPM_CR_INIT_TRX) | CPM_CR_FLG;
		while (cp->cp_cpcr & CPM_CR_FLG);

		/* Set UART mode, 8 bit, no parity, one stop.
		 * Enable receive and transmit.
		 */
		scp->scc_gsmrh = 0;
		scp->scc_gsmrl = 
			(SCC_GSMRL_MODE_UART | SCC_GSMRL_TDCR_16 | SCC_GSMRL_RDCR_16);

		/* Disable all interrupts and clear all pending
		 * events.
		 */
		scp->scc_sccm = 0;
		scp->scc_scce = 0xffff;
		scp->scc_dsr = 0x7e7e;
		scp->scc_psmr = 0x3000;

		scp->scc_gsmrl |= (SCC_GSMRL_ENR | SCC_GSMRL_ENT);

	}
	else {
		up->smc_rbase = dp_addr;	/* Base of receive buffer desc. */
		up->smc_tbase = dp_addr+sizeof(cbd_t);	/* Base of xmt buffer desc. */
		up->smc_rfcr = SMC_EB;
		up->smc_tfcr = SMC_EB;

		/* Set this to 1 for now, so we get single character interrupts.
		*/
		up->smc_mrblr = 1;		/* receive buffer length */
		up->smc_maxidl = 0;		/* wait forever for next char */

		/* Send the CPM an initialize command.
		*/
		chan = smc_chan_map[idx];
		cp->cp_cpcr = mk_cr_cmd(chan, CPM_CR_INIT_TRX) | CPM_CR_FLG;
		printk("%s", "");
		while (cp->cp_cpcr & CPM_CR_FLG);

		/* Set UART mode, 8 bit, no parity, one stop.
		 * Enable receive and transmit.
		 */
		sp->smc_smcmr = smcr_mk_clen(9) |  SMCMR_SM_UART;

		/* And finally, enable Rx and Tx.
		*/
		sp->smc_smcmr |= SMCMR_REN | SMCMR_TEN;
	}

	/* Set up the baud rate generator.
	*/
#ifdef CONFIG_NL1K
	m8xx_cpm_setbrg((ser - rs_table) + 1, bd->bi_baudrate);
	connect_brg(ser, (ser - rs_table) + 1);
#else
	m8xx_cpm_setbrg((ser - rs_table), bd->bi_baudrate);
	connect_brg(ser, (ser - rs_table));
#endif
	
	return 0;
}

#ifdef CONFIG_INPUT_KEYBDEV

void handle_scancode(unsigned char scancode, int down)
{
  printk("handle_scancode(scancode=0x%x, down=%d)\n", scancode, down);
}

static void kbd_bh(unsigned long dummy)
{
}

DECLARE_TASKLET_DISABLED(keyboard_tasklet, kbd_bh, 0);
void (*kbd_ledfunc)(unsigned int led);

#endif
