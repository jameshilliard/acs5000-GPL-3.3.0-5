/*
 * BK Id: SCCS/s.fec.c 1.20 10/11/01 11:55:47 trini
 */
/*
 * Fast Ethernet Controller (FEC) driver for Motorola MPC8xx.
 * Copyright (c) 1997 Dan Malek (dmalek@jlc.net)
 *
 * This version of the driver is specific to the FADS implementation,
 * since the board contains control registers external to the processor
 * for the control of the LevelOne LXT970 transceiver.  The MPC860T manual
 * describes connections using the internal parallel port I/O, which
 * is basically all of Port D.
 *
 * Includes support for the following PHYs: QS6612, LXT970, LXT971/2,
 * AM79C874, BCM5221.
 *
 * Right now, I am very wasteful with the buffers.  I allocate memory
 * pages and then divide them into 2K frame buffers.  This way I know I
 * have buffers large enough to hold one frame within one buffer descriptor.
 * Once I get this working, I will use 64 or 128 byte CPM buffers, which
 * will be much more memory efficient and will easily handle lots of
 * small packets.
 *
 * Much better multiple PHY support by Magnus Damm.
 * Copyright (c) 2000 Ericsson Radio Systems AB.
 *
 * Make use of MII for PHY control configurable.
 * Some fixes.
 * Copyright (c) 2000 Wolfgang Denk, DENX Software Engineering.
 */

//#define	DEBUG_CACHE
//#define	MDIO_DEBUG
//#define	MDIO_DEBUG_FUNC
//#define	MDIO_DEBUG_LINK

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/ptrace.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/bitops.h>
#ifdef CONFIG_FEC_PACKETHOOK
#include <linux/pkthook.h>
#endif
#include <linux/mii.h>

#include <asm/8xx_immap.h>
#include <asm/pgtable.h>
#include <asm/mpc8xx.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/commproc.h>
#include <asm/cacheflush.h>
#include <asm/dma-mapping.h>

/* The PHY support had become a bit unwieldy, so I reorganized it to conform to
 * a consistent set of guidelines. All of the the command tables for specific
 * PHY components now explicitly enable auto-negotiation in their config table
 * and restart an auto-negotiation sequence in their startup table.  If a link-
 * change interrupt is supported by the board, then the PHY will be configured
 * to advertise all capabilities (100FDX, 100HDX, 10FDX, 10HDX).  The MAC
 * duplex will be configured to match the PHY duplex during the link-change
 * interrupt after auto-negotation completes.  For boards that do not support a
 * link-change interrupt, the PHY will be configured to advertise only half-
 * duplex capabilities (100HDX, 10HDX) and the MAC will always be configured
 * for half-duplex operation.
 *
 * An attempt has been made to implement generic PHY support.  If a PHY is
 * detected that does not match any of the specific PHY IDs, then the generic
 * PHY support will be used.  The generic PHY support cannot utilize a
 * link-change interrupt, so the PHY is configured to advertise only half-duplex
 * capabilities.
 *
 * The PHY interrupt is problematic because some boards use a Port C CPM
 * interrupt, some boards use an SIU interrupt, and some boards have no
 * PHY interrupt at all.  Boards that use an SIU interrupt have the
 * PHY_INTERRUPT macro defined in their board-specific header file to be
 * the SIU interrupt vector that they use.  Boards that use a Port C CPM
 * interrupt have the PHY_INTERRUPT_CPM macro defined in their board-specific
 * header file to be the CPM interrupt vector that they use.  All other boards
 * are presumed not to have a PHY interrupt. -- JAL
 */
/* We can't use the PHY interrupt if we aren't using MDIO. */
#ifdef CONFIG_PRxK
#define FEC_INTERRUPT		SIU_LEVEL5
#ifdef CONFIG_FEC_LXT971
#define PHY_INTERRUPT		SIU_IRQ6
#endif
#endif

static int do_release_mii;

#if !defined(CONFIG_USE_MDIO)
#undef PHY_INTERRUPT
#undef PHY_INTERRUPT_CPM
#endif

#undef HAVE_PHY_INTERRUPT
#if defined(PHY_INTERRUPT) || defined(PHY_INTERRUPT_CPM)
#define HAVE_PHY_INTERRUPT
#endif

/* If we have a PHY interrupt, we will advertise both full-duplex and half-
 * duplex capabilities.  If we don't have a PHY interrupt, then we will only
 * advertise half-duplex capabilities.
 */
#define MII_ADVERTISE_HALF	(ADVERTISE_100HALF | ADVERTISE_10HALF | \
				 ADVERTISE_CSMA)
#define MII_ADVERTISE_ALL	(ADVERTISE_100FULL | ADVERTISE_10FULL | \
				 MII_ADVERTISE_HALF)
#ifdef HAVE_PHY_INTERRUPT
#define MII_ADVERTISE_DEFAULT	MII_ADVERTISE_ALL
#else
#define MII_ADVERTISE_DEFAULT	MII_ADVERTISE_HALF
#endif

#ifdef	CONFIG_USE_MDIO
/* Forward declarations of some structures to support different PHYs
*/

typedef struct {
	uint mii_data;
	void (*funct)(uint mii_reg, struct net_device *dev);
} phy_cmd_t;

typedef struct {
	uint mask;
	uint id;
	char *name;

#ifdef CONFIG_PRxK
	int poll_cnt;
	int ix_conf;
	int ix_start;
	phy_cmd_t *config[5];
	phy_cmd_t *startup[2];
#else
	phy_cmd_t *config;
	phy_cmd_t *startup;
#endif
	phy_cmd_t *ack_int;
	phy_cmd_t *shutdown;
} phy_info_t;
#endif	/* CONFIG_USE_MDIO */

/* The number of Tx and Rx buffers.  These are allocated from the page
 * pool.  The code may assume these are power of two, so it is best
 * to keep them that size.
 * We don't need to allocate pages for the transmitter.  We just use
 * the skbuffer directly.
 */
#ifdef CONFIG_ENET_BIG_BUFFERS
#define FEC_ENET_RX_PAGES	16
#define FEC_ENET_RX_FRSIZE	2048
#define FEC_ENET_RX_FRPPG	(PAGE_SIZE / FEC_ENET_RX_FRSIZE)
#define RX_RING_SIZE		(FEC_ENET_RX_FRPPG * FEC_ENET_RX_PAGES)
#define TX_RING_SIZE		16	/* Must be power of two */
#define TX_RING_MOD_MASK	15	/*   for this to work */
#else
#define FEC_ENET_RX_PAGES	4
#define FEC_ENET_RX_FRSIZE	2048
#define FEC_ENET_RX_FRPPG	(PAGE_SIZE / FEC_ENET_RX_FRSIZE)
#define RX_RING_SIZE		(FEC_ENET_RX_FRPPG * FEC_ENET_RX_PAGES)
#define TX_RING_SIZE		8	/* Must be power of two */
#define TX_RING_MOD_MASK	7	/*   for this to work */
#endif

/* Interrupt events/masks.
*/
#define FEC_ENET_HBERR	((uint)0x80000000)	/* Heartbeat error */
#define FEC_ENET_BABR	((uint)0x40000000)	/* Babbling receiver */
#define FEC_ENET_BABT	((uint)0x20000000)	/* Babbling transmitter */
#define FEC_ENET_GRA	((uint)0x10000000)	/* Graceful stop complete */
#define FEC_ENET_TXF	((uint)0x08000000)	/* Full frame transmitted */
#define FEC_ENET_TXB	((uint)0x04000000)	/* A buffer was transmitted */
#define FEC_ENET_RXF	((uint)0x02000000)	/* Full frame received */
#define FEC_ENET_RXB	((uint)0x01000000)	/* A buffer was received */
#define FEC_ENET_MII	((uint)0x00800000)	/* MII interrupt */
#define FEC_ENET_EBERR	((uint)0x00400000)	/* SDMA bus error */

/*
*/
#define FEC_ECNTRL_PINMUX	0x00000004
#define FEC_ECNTRL_ETHER_EN	0x00000002
#define FEC_ECNTRL_RESET	0x00000001

#define FEC_RCNTRL_BC_REJ	0x00000010
#define FEC_RCNTRL_PROM		0x00000008
#define FEC_RCNTRL_MII_MODE	0x00000004
#define FEC_RCNTRL_DRT		0x00000002
#define FEC_RCNTRL_LOOP		0x00000001

#define FEC_TCNTRL_FDEN		0x00000004
#define FEC_TCNTRL_HBC		0x00000002
#define FEC_TCNTRL_GTS		0x00000001

/* Delay to wait for FEC reset command to complete (in us)
*/
#define FEC_RESET_DELAY		50

/* The FEC stores dest/src/type, data, and checksum for receive packets.
 */
#define PKT_MAXBUF_SIZE		1518
#define PKT_MINBUF_SIZE		64
#define PKT_MAXBLR_SIZE		1520

/* The FEC buffer descriptors track the ring buffers.  The rx_bd_base and
 * tx_bd_base always point to the base of the buffer descriptors.  The
 * cur_rx and cur_tx point to the currently available buffer.
 * The dirty_tx tracks the current buffer that is being sent by the
 * controller.  The cur_tx and dirty_tx are equal under both completely
 * empty and completely full conditions.  The empty/ready indicator in
 * the buffer descriptor determines the actual condition.
 */
struct fec_enet_private {
	/* The saved address of a sent-in-place packet/buffer, for skfree(). */
	struct	sk_buff* tx_skbuff[TX_RING_SIZE];
	ushort	skb_cur;
	ushort	skb_dirty;
#ifdef CONFIG_PRxK	
	//[GB]May/06/05  Ethernet Receive Rate Limit
	unsigned int ibr; /* counter for received bytes */
	unsigned long t1; /* used to count time elapsed*/
	unsigned long tlog; /* used to check how much time since the last log issued */
#endif

	/* CPM dual port RAM relative addresses.
	*/
	cbd_t	*rx_bd_base;		/* Address of Rx and Tx buffers. */
	cbd_t	*tx_bd_base;
	cbd_t	*cur_rx, *cur_tx;		/* The next free ring entry */
	cbd_t	*dirty_tx;	/* The ring entries to be free()ed. */
	scc_t	*sccp;
	struct	net_device_stats stats;
	uint	tx_free;
	spinlock_t lock;

	unsigned long *va_rx_base;
	unsigned long *va_tx_base;

	unsigned long *va_rx_cur;
	unsigned long *va_tx_cur;

#ifdef	CONFIG_USE_MDIO
	uint	phy_id;
	uint	phy_id_done;
	uint	phy_status;
	uint	phy_speed;
	phy_info_t	*phy;
	struct work_struct phy_task;
	struct net_device *dev;

	uint	sequence_done;

	uint	phy_addr;
#endif	/* CONFIG_USE_MDIO */

#ifdef CONFIG_FEC_PACKETHOOK
	unsigned long	ph_lock;
	fec_ph_func	*ph_rxhandler;
	fec_ph_func	*ph_txhandler;
	__u16		ph_proto;
	volatile __u32	*ph_regaddr;
	void 		*ph_priv;
#endif
};

static void mii_relink(struct net_device *dev);
static void mii_display_status(struct net_device *dev);
static int fec_enet_open(struct net_device *dev);
static int fec_enet_start_xmit(struct sk_buff *skb, struct net_device *dev);
#ifdef	CONFIG_USE_MDIO
static void fec_enet_mii(struct net_device *dev);
static void fec_enable_mdio(struct net_device *dev);
#endif	/* CONFIG_USE_MDIO */
static irqreturn_t fec_enet_interrupt(int irq, void * dev_id);
#ifdef CONFIG_FEC_PACKETHOOK
static void  fec_enet_tx(struct net_device *dev, __u32 regval);
static void  fec_enet_rx(struct net_device *dev, __u32 regval);
#else
static void  fec_enet_tx(struct net_device *dev);
static void  fec_enet_rx(struct net_device *dev);
#endif
static int fec_enet_close(struct net_device *dev);
static struct net_device_stats *fec_enet_get_stats(struct net_device *dev);
static void set_multicast_list(struct net_device *dev);
static void fec_restart(struct net_device *dev, int duplex);
static void fec_stop(struct net_device *dev);
//this function reenables rx interrupts that were disabled due to flood detection
static void rx_interrupt_reenable(unsigned long targ);

static int MAX_INTERRUPT_WORK = 0;
static int max_interrupt_work;

#ifdef CONFIG_PRxK
/*maximum allowable received bytes per second*/
static unsigned int ibytesec=0; /*0 = disabled*/
#endif

#ifdef CONFIG_PRxK
//timer used to cause a "sleep" in the reception process during the rest of that second
struct timer_list t_rx_reenable;
#endif

////[GB]May/06/05  Ethernet Receive Rate Limit
#ifdef CONFIG_PRxK
//this function reenables rx interrupts that were disabled due to flood detection
static void rx_interrupt_reenable(unsigned long targ);
#endif

/* Transmitter timeout.
*/
#define TX_TIMEOUT (2*HZ)

#ifdef	CONFIG_USE_MDIO
/* MII processing.  We keep this as simple as possible.  Requests are
 * placed on the list (if there is room).  When the request is finished
 * by the MII, an optional function may be called.
 */
typedef struct mii_list {
	uint	mii_regval;
	void	(*mii_func)(uint val, struct net_device *dev);
	struct	mii_list *mii_next;
} mii_list_t;

#define		NMII	64
mii_list_t	mii_cmds[NMII];
mii_list_t	*mii_free;
mii_list_t	*mii_head;
mii_list_t	*mii_tail;

static int	mii_queue(struct net_device *dev, int request,
				void (*func)(uint, struct net_device *));

/* Make MII read/write commands for the FEC.
*/
#define mk_mii_read(REG)	(0x60020000 | (((REG) & 0x1f) << 18))
#define mk_mii_write(REG, VAL)	(0x50020000 | (((REG) & 0x1f) << 18) | \
						((VAL) & 0xffff))
#define mk_mii_end	0

/* values for phy_status */

#define PHY_CONF_ANE	0x0001  /* 1 auto-negotiation enabled */
#define PHY_CONF_LOOP	0x0002  /* 1 loopback mode enabled */
#define PHY_CONF_SPMASK	0x00f0  /* mask for speed */
#define PHY_CONF_10HDX	0x0010  /* 10 Mbit half duplex supported */
#define PHY_CONF_10FDX	0x0020  /* 10 Mbit full duplex supported */
#define PHY_CONF_100HDX	0x0040  /* 100 Mbit half duplex supported */
#define PHY_CONF_100FDX	0x0080  /* 100 Mbit full duplex supported */

#define PHY_STAT_LINK	0x0100  /* 1 up - 0 down */
#define PHY_STAT_FAULT	0x0200  /* 1 remote fault */
#define PHY_STAT_ANC	0x0400  /* 1 auto-negotiation complete	*/
#define PHY_STAT_SPMASK	0xf000  /* mask for speed */
#define PHY_STAT_10HDX	0x1000  /* 10 Mbit half duplex selected	*/
#define PHY_STAT_10FDX	0x2000  /* 10 Mbit full duplex selected	*/
#define PHY_STAT_100HDX	0x4000  /* 100 Mbit half duplex selected */
#define PHY_STAT_100FDX	0x8000  /* 100 Mbit full duplex selected */
#endif	/* CONFIG_USE_MDIO */

#ifdef CONFIG_FEC_PACKETHOOK
int
fec_register_ph(struct net_device *dev, fec_ph_func *rxfun, fec_ph_func *txfun,
		__u16 proto, volatile __u32 *regaddr, void *priv)
{
	struct fec_enet_private *fep;
	int retval = 0;

	fep = dev->priv;

	if (test_and_set_bit(0, (void*)&fep->ph_lock) != 0) {
		/* Someone is messing with the packet hook */
		return -EAGAIN;
	}
	if (fep->ph_rxhandler != NULL || fep->ph_txhandler != NULL) {
		retval = -EBUSY;
		goto out;
	}
	fep->ph_rxhandler = rxfun;
	fep->ph_txhandler = txfun;
	fep->ph_proto = proto;
	fep->ph_regaddr = regaddr;
	fep->ph_priv = priv;

	out:
	fep->ph_lock = 0;

	return retval;
}


int
fec_unregister_ph(struct net_device *dev)
{
	struct fec_enet_private *fep;
	int retval = 0;

	fep = dev->priv;

	if (test_and_set_bit(0, (void*)&fep->ph_lock) != 0) {
		/* Someone is messing with the packet hook */
		return -EAGAIN;
	}

	fep->ph_rxhandler = fep->ph_txhandler = NULL;
	fep->ph_proto = 0;
	fep->ph_regaddr = NULL;
	fep->ph_priv = NULL;

	fep->ph_lock = 0;

	return retval;
}

EXPORT_SYMBOL(fec_register_ph);
EXPORT_SYMBOL(fec_unregister_ph);

#endif /* CONFIG_FEC_PACKETHOOK */


//#define DEBUG_FEC_INTR
//#define DEBUG_FEC_INTR_TX
//#define DEBUG_FEC_INTR_RX

static int
fec_enet_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct fec_enet_private *fep;
	volatile fec_t	*fecp;
	volatile cbd_t	*bdp;

#ifdef DEBUG_FEC_INTR_TX
	dump_buf(skb->data+14, skb->len, 1);
#endif
		
	fep = dev->priv;
	fecp = (volatile fec_t*)dev->base_addr;

	/* Fill in a Tx ring entry */
	bdp = fep->cur_tx;

#ifndef final_version
	if (!fep->tx_free || (bdp->cbd_sc & BD_ENET_TX_READY)) {
		/* Ooops.  All transmit buffers are full.  Bail out.
		 * This should not happen, since the tx queue should be stopped.
		 */
		printk("%s: tx queue full!.\n", dev->name);
		return 1;
	}
#endif

	/* Clear all of the status flags.
	 */
	bdp->cbd_sc &= ~BD_ENET_TX_STATS;

	/* Set buffer length and buffer pointer.
	*/
	bdp->cbd_bufaddr = __pa(skb->data);
	bdp->cbd_datlen = skb->len;

	spin_lock_irq(&fep->lock);

	/* Save skb pointer.
	*/
	fep->tx_skbuff[fep->skb_cur] = skb;

	fep->stats.tx_bytes += skb->len;
	fep->skb_cur = (fep->skb_cur+1) & TX_RING_MOD_MASK;

	/* Push the data cache so the CPM does not get stale memory
	 * data.
	 */
	flush_dcache_range((unsigned long)skb->data,
			   (unsigned long)skb->data + skb->len);


	/* Send it on its way.  Tell FEC its ready, interrupt when done,
	 * its the last BD of the frame, and to put the CRC on the end.
	 */

	bdp->cbd_sc |= (BD_ENET_TX_READY | BD_ENET_TX_INTR
			| BD_ENET_TX_LAST | BD_ENET_TX_TC);

	dev->trans_start = jiffies;

	/* Trigger transmission start */
	fecp->fec_x_des_active = 0x01000000;

	/* If this was the last BD in the ring, start at the beginning again.
	*/
	if (bdp->cbd_sc & BD_ENET_TX_WRAP) {
		bdp = fep->tx_bd_base;
	} else {
		bdp++;
	}

	if (!--fep->tx_free)
		netif_stop_queue(dev);

	fep->cur_tx = (cbd_t *)bdp;

	spin_unlock_irq(&fep->lock);

	return 0;
}

static void
fec_timeout(struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;

	printk("%s: transmit timed out.\n", dev->name);
	fep->stats.tx_errors++;
#ifndef final_version
	{
	int	i;
	cbd_t	*bdp;

	printk("Ring data dump: cur_tx %lx, tx_free %d, dirty_tx %lx, cur_rx %lx\n",
	       (unsigned long)fep->cur_tx, fep->tx_free,
	       (unsigned long)fep->dirty_tx,
	       (unsigned long)fep->cur_rx);

	bdp = fep->tx_bd_base;
	printk(" tx: %u buffers\n",  TX_RING_SIZE);
	for (i = 0 ; i < TX_RING_SIZE; i++) {
		printk("  %08x: %04x %04x %08x\n",
		       (uint) bdp,
		       bdp->cbd_sc,
		       bdp->cbd_datlen,
		       bdp->cbd_bufaddr);
		bdp++;
	}

	bdp = fep->rx_bd_base;
	printk(" rx: %lu buffers\n",  RX_RING_SIZE);
	for (i = 0 ; i < RX_RING_SIZE; i++) {
		printk("  %08x: %04x %04x %08x\n",
		       (uint) bdp,
		       bdp->cbd_sc,
		       bdp->cbd_datlen,
		       bdp->cbd_bufaddr);
		bdp++;
	}
	}
#endif
	if (fep->tx_free)
		netif_wake_queue(dev);
}

/* The interrupt handler.
 * This is called from the MPC core interrupt.
 */
static	irqreturn_t
fec_enet_interrupt(int irq, void * dev_id)
{
	struct	 net_device *dev = dev_id;
	volatile fec_t	*fecp;
	uint	 int_events;
	struct   fec_enet_private *fep = dev->priv;
#ifdef CONFIG_FEC_PACKETHOOK
	__u32 regval;

	if (fep->ph_regaddr) regval = *fep->ph_regaddr;
#endif
	fecp = (volatile fec_t*)dev->base_addr;

#ifdef DEBUG_FEC_INTR
	printk("FEC %s intr.", dev->name);
#endif

	max_interrupt_work = MAX_INTERRUPT_WORK;

	/* Get the interrupt events that caused us to be here.
	*/
	while ((int_events = fecp->fec_ievent) != 0) {
		if (MAX_INTERRUPT_WORK) {
			if (max_interrupt_work > 0)
				max_interrupt_work--;
			else
				break;
		}
		fecp->fec_ievent = int_events;
		if ((int_events & (FEC_ENET_HBERR | FEC_ENET_EBERR)) != 0)
			printk("FEC ERROR %x\n", int_events);
	        if ((int_events & FEC_ENET_BABR) != 0)
	                fep->stats.rx_errors++;
	        if ((int_events & FEC_ENET_BABT) != 0)
			fep->stats.tx_errors++;
		/* Handle receive event in its own function.
		 */
		if (int_events & FEC_ENET_RXF) {
#ifdef DEBUG_FEC_INTR
			printk("R.");
#endif
#ifdef CONFIG_FEC_PACKETHOOK
			fec_enet_rx(dev, regval);
#else
			fec_enet_rx(dev);
#endif
		}

		/* Transmit OK, or non-fatal error. Update the buffer
		   descriptors. FEC handles all errors, we just discover
		   them as part of the transmit process.
		*/
		if (int_events & FEC_ENET_TXF) {
#ifdef DEBUG_FEC_INTR
			printk("T.");
#endif
#ifdef CONFIG_FEC_PACKETHOOK
			fec_enet_tx(dev, regval);
#else
			fec_enet_tx(dev);
#endif
		}

		if (int_events & FEC_ENET_MII) {
#ifdef DEBUG_FEC_INTR
			printk("I.");
#endif
#ifdef	CONFIG_USE_MDIO
			fec_enet_mii(dev);
#else
printk("%s[%d] %s: unexpected FEC_ENET_MII event\n", __FILE__,__LINE__,__FUNCTION__);
#endif	/* CONFIG_USE_MDIO */
		}

	}
#ifdef DEBUG_FEC_INTR
	printk("done");
#endif
	return IRQ_RETVAL(IRQ_HANDLED);
}


static void
#ifdef CONFIG_FEC_PACKETHOOK
fec_enet_tx(struct net_device *dev, __u32 regval)
#else
fec_enet_tx(struct net_device *dev)
#endif
{
	struct	fec_enet_private *fep;
	volatile cbd_t	*bdp;
	struct	sk_buff	*skb;

	fep = dev->priv;
	/* lock while transmitting */
	spin_lock(&fep->lock);
	bdp = fep->dirty_tx;

	if (max_interrupt_work) ++max_interrupt_work;

	while ((bdp->cbd_sc&BD_ENET_TX_READY) == 0) {

		if (MAX_INTERRUPT_WORK) {
			if (max_interrupt_work > 0)
				max_interrupt_work--;
			else
				break;
		}

		if (fep->tx_free == TX_RING_SIZE) break;

#ifdef CONFIG_PRxK
		{
		volatile sysconf8xx_t *psiu = &(((immap_t *)IMAP_ADDR)->im_siu_conf);
		psiu->sc_swsr = 0x556c;
		psiu->sc_swsr = 0xaa39;
		}
#endif


		skb = fep->tx_skbuff[fep->skb_dirty];
		
		/* Check for errors. */
		if (bdp->cbd_sc & (BD_ENET_TX_HB | BD_ENET_TX_LC |
				   BD_ENET_TX_RL | BD_ENET_TX_UN )) {

#ifdef DEBUG_FEC_INTR
			printk("E.");
#endif
			fep->stats.tx_errors++;
			if (bdp->cbd_sc & BD_ENET_TX_HB)  /* No heartbeat */
				fep->stats.tx_heartbeat_errors++;
			if (bdp->cbd_sc & BD_ENET_TX_LC)  /* Late collision */
				fep->stats.tx_window_errors++;
			if (bdp->cbd_sc & BD_ENET_TX_RL)  /* Retrans limit */
				fep->stats.tx_aborted_errors++;
			if (bdp->cbd_sc & BD_ENET_TX_UN)  /* Underrun */
				fep->stats.tx_fifo_errors++;
		} else {
			if ((bdp->cbd_sc & BD_ENET_TX_CSL) &&
			    (fep->phy_status & 
			     (PHY_STAT_100HDX | PHY_STAT_10HDX))) { 
				/* HalfD - Carrier lost */
				fep->stats.tx_errors++;
				fep->stats.tx_carrier_errors++;
			} else {
#ifdef CONFIG_FEC_PACKETHOOK
			/* Packet hook ... */
			if (fep->ph_txhandler &&
			    ((struct ethhdr *)skb->data)->h_proto
			    == fep->ph_proto) {
				fep->ph_txhandler((__u8*)skb->data, skb->len,
						  regval, fep->ph_priv);
			}
#endif
				fep->stats.tx_packets++;
			}
		}

#ifndef final_version
		if (bdp->cbd_sc & BD_ENET_TX_READY)
			printk("HEY! Enet xmit interrupt and TX_READY.\n");
#endif
		/* Deferred means some collisions occurred during transmit,
		 * but we eventually sent the packet OK.
		 */
		if (bdp->cbd_sc & BD_ENET_TX_DEF)
			fep->stats.collisions += 
				(bdp->cbd_sc & BD_ENET_TX_RCMASK)/4;

		/* Free the sk buffer associated with this last transmit.
		 */
#if 0
printk("TXI: %x %x %x\n", bdp, skb, fep->skb_dirty);
#endif
		dev_kfree_skb_irq (skb/*, FREE_WRITE*/);
		fep->tx_skbuff[fep->skb_dirty] = NULL;
		fep->skb_dirty = (fep->skb_dirty + 1) & TX_RING_MOD_MASK;

		/* Update pointer to next buffer descriptor to be transmitted.
		 */
		if (bdp->cbd_sc & BD_ENET_TX_WRAP)
			bdp = fep->tx_bd_base;
		else
			bdp++;

		/* Since we have freed up a buffer, the ring is no longer
		 * full.
		 */
		if (!fep->tx_free++) {
			if (netif_queue_stopped(dev))
				netif_wake_queue(dev);
		}
#ifdef CONFIG_FEC_PACKETHOOK
		/* Re-read register. Not exactly guaranteed to be correct,
		   but... */
		if (fep->ph_regaddr) regval = *fep->ph_regaddr;
#endif
	}
	fep->dirty_tx = (cbd_t *)bdp;
	spin_unlock(&fep->lock);
}

//reenables rx interrupts after the second has expired
//this function is called by a timer initialized inside fec_enet_rx()
void rx_interrupt_reenable(unsigned long targ)
{
	struct  fec_enet_private *fep;
	struct net_device *dev;
	volatile fec_t  *fecp;

	dev = (struct net_device *) targ;
	fecp = (volatile fec_t*)dev->base_addr;
	fep = dev->priv;
	fec_restart(dev,0);
	mii_display_status(dev);
	udelay(1);
}


/* During a receive, the cur_rx points to the current incoming buffer.
 * When we update through the ring, if the next incoming buffer has
 * not been given to the system, we just set the empty indicator,
 * effectively tossing the packet.
 */
static void
#ifdef CONFIG_FEC_PACKETHOOK
fec_enet_rx(struct net_device *dev, __u32 regval)
#else
fec_enet_rx(struct net_device *dev)
#endif
{
	struct	fec_enet_private *fep;
	volatile fec_t	*fecp;
	volatile cbd_t *bdp;
	struct	sk_buff	*skb;
	ushort	pkt_len;
	__u8 *data;

#ifdef CONFIG_PRxK
	//[GB]May/06/05  Ethernet Receive Rate Limit
	unsigned long tdif;
#endif

	fep = dev->priv;
	fecp = (volatile fec_t*)dev->base_addr;

	/* First, grab all of the stats for the incoming packet.
	 * These get messed up if we get called due to a busy condition.
	 */
	bdp = fep->cur_rx;
	data = (__u8*)fep->va_rx_cur;

	if (max_interrupt_work) ++max_interrupt_work;

	while (!(bdp->cbd_sc & BD_ENET_RX_EMPTY)) {

		if (MAX_INTERRUPT_WORK) {
			if (max_interrupt_work > 0)
				max_interrupt_work--;
			else
				break;
		}

#ifdef CONFIG_PRxK
		{
		volatile sysconf8xx_t *psiu = &(((immap_t *)IMAP_ADDR)->im_siu_conf);
		psiu->sc_swsr = 0x556c;
		psiu->sc_swsr = 0xaa39;
		}
#endif

#ifndef final_version
	/* Since we have allocated space to hold a complete frame,
	 * the last indicator should be set.
	 */
	if ((bdp->cbd_sc & BD_ENET_RX_LAST) == 0)
		printk("FEC ENET: rcv is not +last\n");
#endif

	/* Check for errors. */
	if (bdp->cbd_sc & (BD_ENET_RX_LG | BD_ENET_RX_SH | BD_ENET_RX_NO |
			   BD_ENET_RX_CR | BD_ENET_RX_OV)) {
		fep->stats.rx_errors++;
		if (bdp->cbd_sc & BD_ENET_RX_LAST) {
			if (bdp->cbd_sc & (BD_ENET_RX_LG | BD_ENET_RX_SH)) {
			/* Frame too long or too short. */
				fep->stats.rx_length_errors++;
			}
			if (bdp->cbd_sc & BD_ENET_RX_NO) {	/* Frame alignment */
				fep->stats.rx_frame_errors++;
				goto rx_processing_done;
			}
			if (bdp->cbd_sc & BD_ENET_RX_CR)	/* CRC Error */
				fep->stats.rx_crc_errors++;
			if (bdp->cbd_sc & BD_ENET_RX_OV)	/* FIFO overrun */
				fep->stats.rx_crc_errors++;
		}
	}

	/* Report late collisions as a frame error.
	 * On this error, the BD is closed, but we don't know what we
	 * have in the buffer.  So, just drop this frame on the floor.
	 */
	if (bdp->cbd_sc & BD_ENET_RX_CL) {
		fep->stats.rx_errors++;
		fep->stats.rx_frame_errors++;
		goto rx_processing_done;
	}

	/* Process the incoming frame.
	 */
	fep->stats.rx_packets++;
	pkt_len = bdp->cbd_datlen;
	fep->stats.rx_bytes += pkt_len;

#ifdef CONFIG_PRxK
	fep->ibr += pkt_len; /* used to detect exagerate incoming bytes rate */
	if( ibytesec ){ /*if this value is 0, the feature is disabled */
		if( (tdif = (jiffies - fep->t1) / HZ) >= 1){
			fep->t1 = jiffies; /* restarts one second incoming bytes measurement */
			fep->ibr = 0;
		}
		else if(fep->ibr > ibytesec){
			fecp->fec_imask = 0;//disable all interrupts
			//disable receiving related interrupts
			if( (tdif = (jiffies - fep->tlog) ) / HZ >=1){
				printk("Limit of %d incoming bytes reached, dropping packets \
					from now on.",ibytesec);
				printk(" This log will be disabled for 1s to not overload the kernel logger\n");
				fep->tlog = jiffies;
			}
			udelay(1);

			//initialize a timer to reenable rx interrupts at the remainder of this second
			init_timer(&t_rx_reenable);
			t_rx_reenable.expires = jiffies + HZ - (jiffies - fep->t1);//stops for the remainder of that second
			t_rx_reenable.data = (unsigned long) dev; //keeps the pointer to the net device struct
			t_rx_reenable.function = rx_interrupt_reenable; //this function will reenable the interrupt
			add_timer(&t_rx_reenable);

			//zeroes the acumulator for incoming bytes
			//it will make fep->t1 be reinitialized at the first time this is executed after
			//reenabling interrupts
			fep->t1 = jiffies;
			fep->ibr = 0;
			//discard the bytes read
			goto rx_processing_done;
		}
	}
#endif
#ifdef CONFIG_FEC_PACKETHOOK
	/* Packet hook ... */
	if (fep->ph_rxhandler) {
		if (((struct ethhdr *)data)->h_proto == fep->ph_proto) {
			switch (fep->ph_rxhandler(data, pkt_len, regval,
						  fep->ph_priv)) {
			case 1:
				goto rx_processing_done;
				break;
			case 0:
				break;
			default:
				fep->stats.rx_errors++;
				goto rx_processing_done;
			}
		}
	}

	/* If it wasn't filtered - copy it to an sk buffer. */
#endif

	/* This does 16 byte alignment, exactly what we need.
	 * The packet length includes FCS, but we don't want to
	 * include that when passing upstream as it messes up
	 * bridging applications.
	 */
	skb = dev_alloc_skb(pkt_len-4);

	if (skb == NULL) {
		printk("%s: Memory squeeze, dropping packet.\n", dev->name);
		fep->stats.rx_dropped++;
	} else {
		skb_put(skb,pkt_len-4);	/* Make room */
		eth_copy_and_sum(skb,
				 (unsigned char *)data,
				 pkt_len-4, 0);
		skb->protocol=eth_type_trans(skb,dev);
#ifdef DEBUG_FEC_INTR_RX
		dump_buf(skb->data, skb->len, 0);
#endif
		netif_rx(skb);
	}
  rx_processing_done:

	/* Clear the status flags for this buffer.
	*/
	bdp->cbd_sc &= ~BD_ENET_RX_STATS;

	/* Mark the buffer empty.
	*/
	bdp->cbd_sc |= BD_ENET_RX_EMPTY;

	/* Update BD pointer to next entry.
	*/
	if (bdp->cbd_sc & BD_ENET_RX_WRAP) { 
		bdp = fep->rx_bd_base;
		data = (u8*) fep->va_rx_base;
	} else {
		bdp++;
		data += FEC_ENET_RX_FRSIZE;
	}

#if 1
	/* Doing this here will keep the FEC running while we process
	 * incoming frames.  On a heavily loaded network, we should be
	 * able to keep up at the expense of system resources.
	 */
	fecp->fec_r_des_active = 0x01000000;
#endif
#ifdef CONFIG_FEC_PACKETHOOK
	/* Re-read register. Not exactly guaranteed to be correct,
	   but... */
	if (fep->ph_regaddr) regval = *fep->ph_regaddr;
#endif
   } /* while (!(bdp->cbd_sc & BD_ENET_RX_EMPTY)) */
	fep->cur_rx = (cbd_t *)bdp;
	fep->va_rx_cur = (unsigned long*) data;

#if 0
	/* Doing this here will allow us to process all frames in the
	 * ring before the FEC is allowed to put more there.  On a heavily
	 * loaded network, some frames may be lost.  Unfortunately, this
	 * increases the interrupt overhead since we can potentially work
	 * our way back to the interrupt return only to come right back
	 * here.
	 */
	fecp->fec_r_des_active = 0x01000000;
#endif
}


#ifdef	CONFIG_USE_MDIO
static void
fec_enet_mii(struct net_device *dev)
{
	struct	fec_enet_private *fep;
	volatile fec_t	*ep;
	mii_list_t	*mip;
	uint		mii_reg;

	fep = (struct fec_enet_private *)dev->priv;
	ep = &(((immap_t *)IMAP_ADDR)->im_cpm.cp_fec);
	mii_reg = ep->fec_mii_data;

#ifdef MDIO_DEBUG
	printk("fec event 0x%02x mii_reg 0x%04x\n",
		(mii_reg >> 18) & 0x1f, mii_reg&0xffff) ;
#endif

	if ((mip = mii_head) == NULL) {
		printk("MII and no head!\n");
		return;
	}

	do_release_mii = 1;
	if (mip->mii_func != NULL) {
		(*(mip->mii_func))(mii_reg, dev);
	}
	
	if (do_release_mii) {
		mii_head = mip->mii_next;
		mip->mii_next = mii_free;
		mii_free = mip;
		mip = mii_head;
	}


	if (mip != NULL) {
		ep->fec_mii_data = mip->mii_regval;

	}
}

static int
mii_queue(struct net_device *dev, int regval, void (*func)(uint, struct net_device *))
{
	struct fec_enet_private *fep;
	unsigned long	flags;
	mii_list_t	*mip;
	int		retval;

	/* Add PHY address to register command.
	*/
	fep = dev->priv;
	regval |= fep->phy_addr << 23;

	retval = 0;

	/* lock while modifying mii_list */
	spin_lock_irqsave(&fep->lock, flags);

	if ((mip = mii_free) != NULL) {
		mii_free = mip->mii_next;
		mip->mii_regval = regval;
		mip->mii_func = func;
		mip->mii_next = NULL;
		if (mii_head) {
			mii_tail->mii_next = mip;
			mii_tail = mip;
		} else {
			mii_head = mii_tail = mip;
			(&(((immap_t *)IMAP_ADDR)->im_cpm.cp_fec))->fec_mii_data = regval;
		}
	} else {
		retval = 1;
	}

	spin_unlock_irqrestore(&fep->lock, flags);

	return(retval);
}

static void mii_do_cmd(struct net_device *dev, const phy_cmd_t *c)
{
	int k;

	if(!c)
		return;

	for(k = 0; (c+k)->mii_data != mk_mii_end; k++)
		mii_queue(dev, (c+k)->mii_data, (c+k)->funct);
}

static void mii_parse_sr(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;
	volatile uint *s = &(fep->phy_status);

	*s &= ~(PHY_STAT_LINK | PHY_STAT_FAULT | PHY_STAT_ANC);

	if (mii_reg & BMSR_LSTATUS)
		*s |= PHY_STAT_LINK;
	if (mii_reg & BMSR_RFAULT)
		*s |= PHY_STAT_FAULT;
	if (mii_reg & BMSR_ANEGCOMPLETE)
		*s |= PHY_STAT_ANC;

#ifdef MDIO_DEBUG_FUNC
printk("mii_parse_sr phy_status %x\n", *s);
#endif
}

static void mii_check_cr_reset(uint mii_reg, struct net_device *dev)
{
	if (mii_reg & 0x8000) {
		do_release_mii = 0;
	}
}

static void mii_parse_cr(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;
	volatile uint *s = &(fep->phy_status);

	*s &= ~(PHY_CONF_ANE | PHY_CONF_LOOP);

	if (mii_reg & BMCR_ANENABLE)
		*s |= PHY_CONF_ANE;
	if (mii_reg & BMCR_LOOPBACK)
		*s |= PHY_CONF_LOOP;
#ifdef MDIO_DEBUG_FUNC
printk("mii_parse_cr phy_status %x\n", *s);
#endif
}

static void mii_parse_anar(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;
	volatile uint *s = &(fep->phy_status);

	*s &= ~(PHY_CONF_SPMASK);

	if (mii_reg & ADVERTISE_10HALF)
		*s |= PHY_CONF_10HDX;
	if (mii_reg & ADVERTISE_10FULL)
		*s |= PHY_CONF_10FDX;
	if (mii_reg & ADVERTISE_100HALF)
		*s |= PHY_CONF_100HDX;
	if (mii_reg & ADVERTISE_100FULL)
		*s |= PHY_CONF_100FDX;

#ifdef MDIO_DEBUG_FUNC
printk("mii_parse_anar phy_status %x\n", *s);
#endif
}

/* ------------------------------------------------------------------------- */
/* Generic PHY support.  Should work for all PHYs, but does not support link
 * change interrupts.
 */
#ifdef CONFIG_FEC_GENERIC_PHY

static phy_cmd_t generic_config0[] = {
	/* advertise only half-duplex capabilities */
	{ mk_mii_write(MII_ADVERTISE, MII_ADVERTISE_HALF), mii_parse_anar },

	/* enable auto-negotiation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t generic_startup0[] = {
	/* restart auto-negotiation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART), NULL },
	{ mk_mii_end, }
};

static phy_info_t phy_info_generic = {
	0xffffffff,
	0x00000000, /* 0-->match any PHY */
	"GENERIC",

#ifdef CONFIG_PRxK
	0, 0, 0,
	{ generic_config0 },
	{ generic_startup0 },
#else
	generic_config0,
	generic_startup0,
#endif
	(phy_cmd_t []) { /* ack_int */
		/* We don't actually use the ack_int table with a generic
		 * PHY, but putting a reference to mii_parse_sr here keeps
		 * us from getting a compiler warning about unused static
		 * functions in the case where we only compile in generic
		 * PHY support.
		 */
		{ mk_mii_read(MII_BMSR), mii_parse_sr },
		{ mk_mii_end, }
	},
	(phy_cmd_t []) {  /* shutdown */
		{ mk_mii_end, }
	},
};
#endif	/* ifdef CONFIG_FEC_GENERIC_PHY */

/* ------------------------------------------------------------------------- */
/* The Level one LXT970 is used by many boards				     */

#ifdef CONFIG_FEC_LXT970

#define MII_LXT970_MIRROR    16  /* Mirror register           */
#define MII_LXT970_IER       17  /* Interrupt Enable Register */
#define MII_LXT970_ISR       18  /* Interrupt Status Register */
#define MII_LXT970_CONFIG    19  /* Configuration Register    */
#define MII_LXT970_CSR       20  /* Chip Status Register      */

static void mii_parse_lxt970_csr(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;
	volatile uint *s = &(fep->phy_status);

	*s &= ~(PHY_STAT_SPMASK | PHY_STAT_LINK | PHY_STAT_ANC);

	if (mii_reg & 0x2000)
		*s |= PHY_STAT_LINK;
	if (mii_reg & 0x0200)
		*s |= PHY_STAT_ANC;

	if (mii_reg & 0x0800) {
		if (mii_reg & 0x1000)
			*s |= PHY_STAT_100FDX;
		else
			*s |= PHY_STAT_100HDX;
	}
	else {
		if (mii_reg & 0x1000)
			*s |= PHY_STAT_10FDX;
		else
			*s |= PHY_STAT_10HDX;
	}
}

static phy_cmd_t lxt970_config0[] = {
	/* configure link capabilities to advertise */
	{ mk_mii_write(MII_ADVERTISE, MII_ADVERTISE_DEFAULT), mii_parse_anar },

	/* enable auto-negotiation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE), mii_parse_cr },
	{ mk_mii_end, },
};

static phy_cmd_t lxt970_startup0[] = {
	/* startup - enable interrupts */
	{ mk_mii_write(MII_LXT970_IER, 0x0002), NULL },

	/* restart auto-negotiation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART), NULL },
	{ mk_mii_end, }
};

static phy_info_t phy_info_lxt970 = {
	0xffffffff,
	0x07810000,
	"LXT970",

#ifdef CONFIG_PRxK
	0, 0, 0,
	{ lxt970_config0 },
	{ lxt970_startup0 },
#else
	lxt970_config0,
	lxt970_startup0,
#endif
	(phy_cmd_t []) { /* ack_int */
		/* read SR and ISR to acknowledge */
		{ mk_mii_read(MII_BMSR), NULL },
		{ mk_mii_read(MII_BMSR), mii_parse_sr },
		{ mk_mii_read(MII_LXT970_ISR), NULL },

		/* find out the current status */
		{ mk_mii_read(MII_LXT970_CSR), mii_parse_lxt970_csr },
		{ mk_mii_end, }
	},
	(phy_cmd_t []) {  /* shutdown - disable interrupts */
		{ mk_mii_write(MII_LXT970_IER, 0x0000), NULL },
		{ mk_mii_end, }
	},
};

#endif /* CONFIG_FEC_LXT970 */

/* ------------------------------------------------------------------------- */
/* The Level one LXT971 is used on some of my custom boards                  */

#ifdef CONFIG_FEC_LXT971

/* register definitions for the 971 */

#define MII_LXT971_PCR       16  /* Port Control Register     */
#define MII_LXT971_SR2       17  /* Status Register 2         */
#define MII_LXT971_IER       18  /* Interrupt Enable Register */
#define MII_LXT971_ISR       19  /* Interrupt Status Register */
#define MII_LXT971_LCR       20  /* LED Control Register      */
#define MII_LXT971_TCR       30  /* Transmit Control Register */

/*
 * I had some nice ideas of running the MDIO faster...
 * The 971 should support 8MHz and I tried it, but things acted really
 * weird, so 2.5 MHz ought to be enough for anyone...
 */

static void mii_parse_lxt971_sr2(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;
	volatile uint *s = &(fep->phy_status);

	*s &= ~(PHY_STAT_SPMASK | PHY_STAT_LINK | PHY_STAT_ANC
		| PHY_STAT_FAULT);

	if (mii_reg & 0x0400)
		*s |= PHY_STAT_LINK;
	if (mii_reg & 0x0080)
		*s |= PHY_STAT_ANC;

	if (mii_reg & 0x4000) {
		if (mii_reg & 0x0200)
			*s |= PHY_STAT_100FDX;
		else
			*s |= PHY_STAT_100HDX;
	}
	else {
		if (mii_reg & 0x0200)
			*s |= PHY_STAT_10FDX;
		else
			*s |= PHY_STAT_10HDX;
	}
	if (mii_reg & 0x0008)
		*s |= PHY_STAT_FAULT;

#ifdef MDIO_DEBUG_FUNC
printk("mii_parse_lxt971_sr2 phy_status %x\n", *s);
#endif
}

static void mii_print_status(uint mii_reg, struct net_device *dev)
{
#ifdef MDIO_DEBUG_FUNC
printk("mii_print_status\n");
#endif

	mii_display_status(dev);
}

phy_cmd_t phy_cmd_status[] = {
	/* find out the current status */
	{ mk_mii_read(MII_BMSR), mii_parse_sr },
	{ mk_mii_read(MII_LXT971_SR2), mii_parse_lxt971_sr2 },
	{ mk_mii_read(MII_BMSR), mii_print_status },
	{ mk_mii_end, }
};

static void mii_parse_lxt971_isr(uint mii_reg, struct net_device *dev)
{
#ifdef MDIO_DEBUG_FUNC
printk("mii_parse_lxt971_isr\n");
#endif

	mii_do_cmd(dev, phy_cmd_status);
}

#ifdef CONFIG_PRxK

#define LXT971_LED_MASK 0x3d00

static phy_cmd_t lxt971_config0[] = {
	/* configure link capabilities to advertise */
	{ mk_mii_write(MII_ADVERTISE, MII_ADVERTISE_DEFAULT), mii_parse_anar },
	/* enable auto-negotiation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE), mii_parse_cr },

	/* set leds functions */
	{ mk_mii_write(MII_LXT971_LCR, LXT971_LED_MASK), NULL },

	{ mk_mii_end, }
};

static phy_cmd_t lxt971_config4[] = {
	{ mk_mii_read(MII_BMCR), mii_check_cr_reset },
	{ mk_mii_write(MII_ADVERTISE, ADVERTISE_10HALF), mii_parse_anar },
	{ mk_mii_write(MII_BMCR, 0), mii_parse_cr },
	{ mk_mii_write(MII_LXT971_LCR, LXT971_LED_MASK), NULL },
	{ mk_mii_end, }
};

static phy_cmd_t lxt971_config1[] = {
	{ mk_mii_read(MII_BMCR), mii_check_cr_reset },
	{ mk_mii_write(MII_ADVERTISE, ADVERTISE_100HALF), mii_parse_anar },
	{ mk_mii_write(MII_BMCR, BMCR_SPEED100), mii_parse_cr },
	{ mk_mii_write(MII_LXT971_LCR, LXT971_LED_MASK), NULL },
	{ mk_mii_end, }
};

static phy_cmd_t lxt971_config2[] = {
	{ mk_mii_read(MII_BMCR), mii_check_cr_reset },
	{ mk_mii_write(MII_ADVERTISE, ADVERTISE_10FULL), mii_parse_anar },
	{ mk_mii_write(MII_BMCR, BMCR_FULLDPLX), mii_parse_cr },
	{ mk_mii_write(MII_LXT971_LCR, LXT971_LED_MASK), NULL },
	{ mk_mii_end, }
};

static phy_cmd_t lxt971_config3[] = {
	{ mk_mii_read(MII_BMCR), mii_check_cr_reset },
	{ mk_mii_write(MII_ADVERTISE, ADVERTISE_100FULL), mii_parse_anar },
	{ mk_mii_write(MII_BMCR, BMCR_SPEED100 | BMCR_FULLDPLX), mii_parse_cr },
	{ mk_mii_write(MII_LXT971_LCR, LXT971_LED_MASK), NULL },
	{ mk_mii_end, }
};

static phy_cmd_t lxt971_startup0[] = {
	{ mk_mii_write(MII_LXT971_IER, 0x00f2), NULL },

	/* restart auto-negotiation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART), NULL },
	{ mk_mii_end, }
};

static phy_cmd_t lxt971_startup1[] = {
	{ mk_mii_write(MII_LXT971_IER, 0x00f2), NULL },
	{ mk_mii_end, }
};

#endif

static phy_info_t phy_info_lxt971 = {
	0xfffffff0,
	0x001378e0,
	"LXT971/2",

#ifdef CONFIG_PRxK
	0, 0, 0,
	{ lxt971_config0, lxt971_config1, lxt971_config2,  
	  lxt971_config3, lxt971_config4 },
	{ lxt971_startup0, lxt971_startup1 },
#else
	(phy_cmd_t []) {  /* config */
		/* configure link capabilities to advertise */
		{ mk_mii_write(MII_ADVERTISE, MII_ADVERTISE_DEFAULT), mii_parse_anar },
		/* enable auto-negotiation */
		{ mk_mii_write(MII_BMCR, BMCR_ANENABLE), mii_parse_cr },
		{ mk_mii_end, }
	},
	(phy_cmd_t []) {  /* startup - enable interrupts */
		{ mk_mii_write(MII_LXT971_IER, 0x00f2), NULL },

		/* restart auto-negotiation */
		{ mk_mii_write(MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART), NULL },
		{ mk_mii_end, }
	},
#endif
	(phy_cmd_t []) { /* ack_int */
		/* we only need to read ISR to acknowledge */
		{ mk_mii_read(MII_LXT971_ISR), mii_parse_lxt971_isr },
		{ mk_mii_end, }
	},
	(phy_cmd_t []) {  /* shutdown - disable interrupts */
		{ mk_mii_write(MII_LXT971_IER, 0x0000), NULL },
		{ mk_mii_end, }
	},
};

phy_cmd_t ack_int1[] = {
	/* we only need to read ISR to acknowledge */
	{ mk_mii_read(MII_LXT971_ISR), NULL },
	{ mk_mii_end, }
};

#endif /* CONFIG_FEC_LXT971 */


#ifdef CONFIG_FEC_DP83846A

#define MII_DP83846A_PHYSTS       16  /* PHY Status Register */
#define MII_DP83846A_10BTSCR      26  /* 10Base-T Status/Control Register */

static void mii_parse_dp83846a_physts(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;
	volatile uint *s = &(fep->phy_status);

	*s &= ~(PHY_STAT_SPMASK | PHY_STAT_LINK | PHY_STAT_ANC
		| PHY_STAT_FAULT);

	if (mii_reg & 0x0001)
		*s |= PHY_STAT_LINK;
	if (mii_reg & 0x0010)
		*s |= PHY_STAT_ANC;

	if (mii_reg & 0x0002) {
		if (mii_reg & 0x0004)
			*s |= PHY_STAT_10FDX;
		else
			*s |= PHY_STAT_10HDX;
	}
	else {
		if (mii_reg & 0x0004)
			*s |= PHY_STAT_100FDX;
		else
			*s |= PHY_STAT_100HDX;
	}
	if (mii_reg & 0x0040)
		*s |= PHY_STAT_FAULT;

#ifdef MDIO_DEBUG_FUNC
printk("mii_parse_dp83846a_physts phy_status %x\n", *s);
#endif
}

#ifdef CONFIG_PRxK

static phy_cmd_t dp8348_config0[] = {
	/* configure link capabilities to advertise */
	{ mk_mii_write(MII_ADVERTISE, MII_ADVERTISE_DEFAULT), mii_parse_anar },
	/* enable auto-negotiation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t dp8348_config4[] = {
	{ mk_mii_write(MII_ADVERTISE, ADVERTISE_10HALF), mii_parse_anar },
	{ mk_mii_write(MII_BMCR, 0), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t dp8348_config1[] = {
	{ mk_mii_write(MII_ADVERTISE, ADVERTISE_100HALF), mii_parse_anar },
	{ mk_mii_write(MII_BMCR, BMCR_SPEED100), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t dp8348_config3[] = {
	{ mk_mii_write(MII_ADVERTISE, ADVERTISE_10FULL), mii_parse_anar },
	{ mk_mii_write(MII_BMCR, BMCR_FULLDPLX), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t dp8348_config2[] = {
	{ mk_mii_write(MII_ADVERTISE, ADVERTISE_100FULL), mii_parse_anar },
	{ mk_mii_write(MII_BMCR, BMCR_SPEED100 | BMCR_FULLDPLX), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t dp8348_startup0[] = {
	/* restart auto-negotiation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART), NULL },
	{ mk_mii_end, }
};

static phy_cmd_t dp8348_startup1[] = {
	{ mk_mii_end, }
};

#endif

static phy_info_t phy_info_dp83846a = {
	0xfffffff0,
	0x20005c20,
	"DP83846A",

#ifdef CONFIG_PRxK
	2, 0, 0,
	{ dp8348_config0, dp8348_config1, dp8348_config2,
	  dp8348_config3, dp8348_config4 },
	{ dp8348_startup0, dp8348_startup1 },
#else
	(phy_cmd_t []) {  /* config */
		/* configure link capabilities to advertise */
		{ mk_mii_write(MII_ADVERTISE, MII_ADVERTISE_DEFAULT), mii_parse_anar },
		/* enable auto-negotiation */
		{ mk_mii_write(MII_BMCR, BMCR_ANENABLE), mii_parse_cr },
		{ mk_mii_end, }
	},
	(phy_cmd_t []) {  /* startup - enable interrupts */
		/* restart auto-negotiation */
		{ mk_mii_write(MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART), NULL },
		{ mk_mii_end, }
	},
#endif
	(phy_cmd_t []) { /* ack_int */
		/* find out the current status */
		{ mk_mii_read(MII_BMSR), mii_parse_sr },
		{ mk_mii_read(MII_DP83846A_PHYSTS), mii_parse_dp83846a_physts },
		{ mk_mii_end, }
	},
	(phy_cmd_t []) {  /* shutdown - disable interrupts */
		{ mk_mii_end, }
	},
};

#endif /* CONFIG_FEC_DP83846A */


/* The KV8995XA is an Ethernet switch chip used in the KVMnet board */
#ifdef CONFIG_FEC_KS8995

#ifdef CONFIG_PRxK

static phy_cmd_t ks8995_config0[] = {
	{ mk_mii_write(MII_ADVERTISE, MII_ADVERTISE_DEFAULT), mii_parse_anar },
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t ks8995_config4[] = {
	{ mk_mii_write(MII_ADVERTISE, ADVERTISE_10HALF), mii_parse_anar },
	{ mk_mii_write(MII_BMCR, 0), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t ks8995_config1[] = {
	{ mk_mii_write(MII_ADVERTISE, ADVERTISE_100HALF), mii_parse_anar },
	{ mk_mii_write(MII_BMCR, BMCR_SPEED100), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t ks8995_config3[] = {
	{ mk_mii_write(MII_ADVERTISE, ADVERTISE_10FULL), mii_parse_anar },
	{ mk_mii_write(MII_BMCR, BMCR_FULLDPLX), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t ks8995_config2[] = {
	{ mk_mii_write(MII_ADVERTISE, ADVERTISE_100FULL), mii_parse_anar },
	{ mk_mii_write(MII_BMCR, BMCR_SPEED100 | BMCR_FULLDPLX), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t ks8995_startup0[] = {
	/* restart auto-negotiation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART), NULL },
	{ mk_mii_end, }
};

static phy_cmd_t ks8995_startup1[] = {
	{ mk_mii_end, }
};

#endif

static phy_info_t phy_info_ks8995 = {
	0xffffffff,
	0x00221450,
	"KS8995",

#ifdef CONFIG_PRxK
	2, 0, 0,
	{ ks8995_config0, ks8995_config1, ks8995_config2,
	  ks8995_config3, ks8995_config4 },
	{ ks8995_startup0, ks8995_startup1 },
#else
	(phy_cmd_t []) {  /* config */
		{ mk_mii_write(MII_ADVERTISE, MII_ADVERTISE_DEFAULT), 
			mii_parse_anar },
		{ mk_mii_write(MII_BMCR, BMCR_ANENABLE), mii_parse_cr },
		{ mk_mii_end, }
	},
	(phy_cmd_t []) {  /* startup - enable interrupts */
		/* restart auto-negotiation */
		{ mk_mii_write(MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART), 
			NULL },
		{ mk_mii_end, }
	},
#endif
	(phy_cmd_t []) { /* ack_int */
		/* find out the current status */
		{ mk_mii_read(MII_BMSR), mii_parse_sr },
		{ mk_mii_end, }
	},
	(phy_cmd_t []) {  /* shutdown - disable interrupts */
		{ mk_mii_end, }
	},
};

#endif /* CONFIG_FEC_KS8995 */

/* ------------------------------------------------------------------------- */
/* The Quality Semiconductor QS6612 is used on the RPX CLLF                  */

/* ------------------------------------------------------------------------- */
/* The Quality Semiconductor QS6612 is used on the RPX CLLF                  */

#ifdef CONFIG_FEC_QS6612

/* register definitions */

#define MII_QS6612_MCR       17  /* Mode Control Register      */
#define MII_QS6612_FTR       27  /* Factory Test Register      */
#define MII_QS6612_MCO       28  /* Misc. Control Register     */
#define MII_QS6612_ISR       29  /* Interrupt Source Register  */
#define MII_QS6612_IMR       30  /* Interrupt Mask Register    */
#define MII_QS6612_PCR       31  /* 100BaseTx PHY Control Reg. */

static void mii_parse_qs6612_pcr(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;
	volatile uint *s = &(fep->phy_status);

	*s &= ~(PHY_STAT_SPMASK);

	switch((mii_reg >> 2) & 7) {
	case 1: *s |= PHY_STAT_10HDX; break;
	case 2: *s |= PHY_STAT_100HDX; break;
	case 5: *s |= PHY_STAT_10FDX; break;
	case 6: *s |= PHY_STAT_100FDX; break;
	}
}

static phy_cmd_t qs6612_config0[] = {
	/* The PHY powers up isolated on the RPX,
	 * so send a command to allow operation.
	 */
	{ mk_mii_write(MII_QS6612_PCR, 0x0dc0), NULL },

	/* configure link capabilities to advertise */
	{ mk_mii_write(MII_ADVERTISE, MII_ADVERTISE_DEFAULT), mii_parse_anar },

	/* enable auto-negotiation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t qs6612_startup0[] = {
	/* startup - enable interrupts */
	{ mk_mii_write(MII_QS6612_IMR, 0x0050), NULL },

	/* restart auto-negotation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART), NULL },
	{ mk_mii_end, }
};

static phy_info_t phy_info_qs6612 = {
	0xffffffff,
	0x00181440,
	"QS6612",

#ifdef CONFIG_PRxK
	0, 0, 0,
	{ qs6612_config0 },
	{ qs6612_startup0 },
#else
	qs6612_config0,
	qs6612_startup0,
#endif
	(phy_cmd_t []) { /* ack_int */
		/* we need to read ISR, SR and ANER to acknowledge */
		{ mk_mii_read(MII_QS6612_ISR), NULL },
		{ mk_mii_read(MII_BMSR), NULL },
		{ mk_mii_read(MII_BMSR), mii_parse_sr },
		{ mk_mii_read(MII_EXPANSION), NULL },

		/* read pcr to get info */
		{ mk_mii_read(MII_QS6612_PCR), mii_parse_qs6612_pcr },
		{ mk_mii_end, }
	},
	(phy_cmd_t []) {  /* shutdown - disable interrupts */
		{ mk_mii_write(MII_QS6612_IMR, 0x0000), NULL },
		{ mk_mii_end, }
	},
};

#endif /* CONFIG_FEC_QS6612 */

/* -------------------------------------------------------------------------- */
/* AMD Am79C874                                                               */

#ifdef CONFIG_FEC_AM79C874

#define MII_AM79C874_DIAG	18	/* diagnostic register */
#define MII_AM79C874_ICSR	17	/* interrupt control and status
					 * register */

static void mii_parse_am79c874_diag(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;
	volatile uint *s = &(fep->phy_status);

	*s &= ~PHY_STAT_SPMASK;

	if (mii_reg & 0x0400) {
		if (mii_reg & 0x0800)
			*s |= PHY_STAT_100FDX;
		else
			*s |= PHY_STAT_100HDX;
	}
	else {
		if (mii_reg & 0x0800)
			*s |= PHY_STAT_10FDX;
		else
			*s |= PHY_STAT_10HDX;
	}
}

static phy_cmd_t am79c874_config0[] = {
	/* configure link capabilities to advertise */
	{ mk_mii_write(MII_ADVERTISE, MII_ADVERTISE_DEFAULT), mii_parse_anar },

	/* enable auto-negotiation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t am79c874_startup0[] = {
	/* startup - enable interrupts */
	{ mk_mii_write(MII_AM79C874_ICSR, 0x0500), NULL },

	/* restart auto-negotation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART), NULL },
	{ mk_mii_end, }
};

static phy_info_t phy_info_am79c874 = {
	0xffffffff,
	0x00022561,
	"Am79C874",

#ifdef CONFIG_PRxK
	0, 0, 0,
	{ am79c874_config0 },
	{ am79c874_startup0 },
#else
	am79c874_config0,
	am79c874_startup0,
#endif
	(phy_cmd_t []) {	/* ack_int */
		/* Read SR and ICSR to acknowledge */
		{ mk_mii_read(MII_BMSR), NULL },
		{ mk_mii_read(MII_BMSR), mii_parse_sr },
		{ mk_mii_read(MII_AM79C874_ICSR), NULL },

		/* find out the current link status */
		{ mk_mii_read(MII_AM79C874_DIAG), mii_parse_am79c874_diag },
		{ mk_mii_end, }
	},
	(phy_cmd_t []) {	/* shutdown - disable interrupts */
		{ mk_mii_write(MII_AM79C874_ICSR, 0x0000), NULL },
		{ mk_mii_end, }
	},
};

#endif	/* CONFIG_FEC_AM79C874 */

/* ------------------------------------------------------------------------- */
/* Broadcom BCM5221				     			     */

#ifdef CONFIG_FEC_BCM5221

#define	MII_BCM5221_AUXCS	24	/* auxiliary control and status */
#define	MII_BCM5221_INTR	26	/* interrupt register */
#define	MII_BCM5221_TEST	31	/* test register */
#define	MII_BCM5221_SHADOW_AUX4	26	/* auxiliary mode 4 (shadow register) */

static void mii_parse_bcm5221_auxcs(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;
	volatile uint *s = &(fep->phy_status);

	*s &= ~PHY_STAT_SPMASK;

	if (mii_reg & 0x0002) {
		if (mii_reg & 0x0001)
			*s |= PHY_STAT_100FDX;
		else
			*s |= PHY_STAT_100HDX;
	}
	else {
		if (mii_reg & 0x0001)
			*s |= PHY_STAT_10FDX;
		else
			*s |= PHY_STAT_10HDX;
	}
}

static phy_cmd_t bcm5221_config0[] = {
	/* Workaround for link LED bug in BCM5221 Rev. A3.
	 * This workaround is certified by Broadcom to be
	 * harmless in subsequent revisions.
	 */
	{ mk_mii_write(MII_BCM5221_TEST, 0x008b), NULL },
	{ mk_mii_write(MII_BCM5221_SHADOW_AUX4, 0x3008), NULL },
	{ mk_mii_write(MII_BCM5221_TEST, 0x000b), NULL },

	/* configure link capabilities to advertise */
	{ mk_mii_write(MII_ADVERTISE, MII_ADVERTISE_DEFAULT), mii_parse_anar },

	/* enable auto-negotiation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE), mii_parse_cr },
	{ mk_mii_end, }
};

static phy_cmd_t bcm5221_startup0[] = {
	{ mk_mii_write(MII_BCM5221_INTR, 0x4000), NULL },

	/* restart auto-negotation */
	{ mk_mii_write(MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART), NULL },
	{ mk_mii_end, }
};

static phy_info_t phy_info_bcm5221 = {
	0xffffffff,
	0x0004061E,
	"BCM5221",

#ifdef CONFIG_PRxK
	0, 0, 0,
	{ bcm5221_config0 },
	{ bcm5221_startup0 },
#else
	bcm5221_config0,
	bcm5221_startup0,
#endif
	(phy_cmd_t []) {	/* ack_int */
		/* read SR and ISR to acknowledge */
		{ mk_mii_read(MII_BMSR), NULL },
		{ mk_mii_read(MII_BMSR), mii_parse_sr },
		{ mk_mii_read(MII_BCM5221_INTR), NULL },

		/* find out the current link status */
		{ mk_mii_read(MII_BCM5221_AUXCS), mii_parse_bcm5221_auxcs },
		{ mk_mii_end, }
	},
	(phy_cmd_t []) {	/* shutdown - disable interrupts */
		{ mk_mii_write(MII_BCM5221_INTR, 0x0f00), NULL },
		{ mk_mii_end, }
	},
};

#endif /* CONFIG_FEC_BCM5221 */


static phy_info_t *phy_info[] = {

#ifdef CONFIG_FEC_LXT970
	&phy_info_lxt970,
#endif /* CONFIG_FEC_LXT970 */

#ifdef CONFIG_FEC_LXT971
	&phy_info_lxt971,
#endif /* CONFIG_FEC_LXT971 */

#ifdef CONFIG_FEC_DP83846A
	&phy_info_dp83846a,
#endif /* CONFIG_FEC_DP83846A */
	
#ifdef CONFIG_FEC_QS6612
	&phy_info_qs6612,
#endif /* CONFIG_FEC_QS6612 */

#ifdef CONFIG_FEC_AM79C874
	&phy_info_am79c874,
#endif /* CONFIG_FEC_AM79C874 */

#ifdef CONFIG_FEC_BCM5221
	&phy_info_bcm5221,
#endif /* CONFIG_FEC_BCM5221 */

#ifdef CONFIG_FEC_KS8995
	&phy_info_ks8995,
#endif /* CONFIG_FEC_KS8995 */
#ifdef CONFIG_FEC_GENERIC_PHY
	/* Generic PHY support.  This must be the last PHY in the table.
	 * It will be used to support any PHY that doesn't match a previous
	 * entry in the table.
	 */
	&phy_info_generic,
#endif /* CONFIG_FEC_GENERIC_PHY */

	NULL
};


static void mii_display_status(struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;
	volatile uint *s = &(fep->phy_status);

	printk("%s: status: ", dev->name);

	if (*s & PHY_STAT_LINK) {
		printk("link up");
		netif_carrier_on(dev);
		switch(*s & PHY_STAT_SPMASK) {
		case PHY_STAT_100FDX: printk(", 100 Mbps Full Duplex"); break;
		case PHY_STAT_100HDX: printk(", 100 Mbps Half Duplex"); break;
		case PHY_STAT_10FDX: printk(", 10 Mbps Full Duplex"); break;
		case PHY_STAT_10HDX: printk(", 10 Mbps Half Duplex"); break;
		default:
			printk(", Unknown speed/duplex");
		}

		if (*s & PHY_STAT_ANC) 
			printk(", auto-negotiation complete");
		
	} else {
		netif_carrier_off(dev);
		printk("link down");
	}

	if (*s & PHY_STAT_FAULT)
		printk(", remote fault");

#ifdef DEBUG_CACHE
	{
		volatile uint cst, dst;
		__asm__ __volatile__ ("mfspr %0, 560" : "=r" (cst));
		__asm__ __volatile__ ("mfspr %0, 568" : "=r" (dst));
		printk(" (D=%08x C=%08x)", dst, cst);
	}
#endif

	printk(".\n");
}

static void mii_display_config(struct work_struct *work)
{
	struct fec_enet_private *fep =
		container_of(work, struct fec_enet_private, phy_task);
	struct net_device *dev = fep->dev;
	volatile uint *s = &(fep->phy_status);

	printk("%s: config: auto-negotiation ", dev->name);

	if (*s & PHY_CONF_ANE)
		printk("on");
	else
		printk("off");

	if (*s & PHY_CONF_100FDX)
		printk(", 100FDX");
	if (*s & PHY_CONF_100HDX)
		printk(", 100HDX");
	if (*s & PHY_CONF_10FDX)
		printk(", 10FDX");
	if (*s & PHY_CONF_10HDX)
		printk(", 10HDX");
	if (!(*s & PHY_CONF_SPMASK))
		printk(", No speed/duplex selected?");

	if (*s & PHY_CONF_LOOP)
		printk(", loopback enabled");
	if (MAX_INTERRUPT_WORK) {
		printk(", Intr_Max %d", MAX_INTERRUPT_WORK);
	}

	printk(".\n");

	fep->sequence_done = 1;
}

static void mii_relink(struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;
	int duplex = 0;
	volatile fec_t	*fecp = &(((immap_t *)IMAP_ADDR)->im_cpm.cp_fec);

	if (fep->phy_status & (PHY_STAT_100FDX | PHY_STAT_10FDX))
		duplex = 1;
	if (duplex) {
		fecp->fec_r_cntrl = FEC_RCNTRL_MII_MODE;	/* MII enable */
		fecp->fec_x_cntrl = FEC_TCNTRL_FDEN;		/* FD enable */
	} else {
		fecp->fec_r_cntrl = FEC_RCNTRL_MII_MODE | FEC_RCNTRL_DRT;
		fecp->fec_x_cntrl = 0;
	}

#ifdef MDIO_DEBUG_LINK
	printk("  mii_relink:  duplex=%d\n", duplex);
#endif
}

static void mii_queue_config(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;

	fep->dev = dev;
	INIT_WORK(&fep->phy_task, mii_display_config);
	schedule_work(&fep->phy_task);
}

phy_cmd_t phy_cmd_config[] = { { mk_mii_read(MII_BMCR), mii_queue_config },
			       { mk_mii_end, } };

/* Read remainder of PHY ID.
*/
static void
mii_discover_phy3(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep;
	int	i;

//printk("fec mii_phy3 0x%02x mii_reg 0x%04x\n", (mii_reg >> 18) & 0x1f, mii_reg&0xffff) ;
	fep = dev->priv;
	fep->phy_id |= (mii_reg & 0xffff);

	for(i = 0; phy_info[i]; i++)
		if (phy_info[i]->id == (fep->phy_id & phy_info[i]->mask))
			break;

	if(!phy_info[i])
		panic("%s: PHY id 0x%08x is not supported!\n",
		      dev->name, fep->phy_id);

	fep->phy = phy_info[i];
	fep->phy_id_done = 1;

	printk("%s: Phy @ 0x%x, type %s (0x%08x)\n",
		dev->name, fep->phy_addr, fep->phy->name, fep->phy_id);
}

/* Scan all of the MII PHY addresses looking for someone to respond
 * with a valid ID.  This usually happens quickly.
 */
static void
mii_discover_phy(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep;
	uint	phytype;

	fep = dev->priv;
//printk("fec mii_phy 0x%02x mii_reg 0x%04x\n", (mii_reg >> 18) & 0x1f, mii_reg&0xffff) ;

	if ((phytype = (mii_reg & 0xffff)) != 0xffff) {

		/* Got first part of ID, now get remainder.
		*/
		fep->phy_id = phytype << 16;
		mii_queue(dev, mk_mii_read(MII_PHYSID2), mii_discover_phy3);
	} else {
		fep->phy_addr++;
		if (fep->phy_addr < 32) {
			mii_queue(dev, mk_mii_read(MII_PHYSID1),
							mii_discover_phy);
		} else {
			printk("fec: No PHY device found.\n");
		}
	}
}
#endif	/* CONFIG_USE_MDIO */

#ifdef HAVE_PHY_INTERRUPT
/* This interrupt occurs when the PHY detects a link change.
*/
static irqreturn_t
#ifdef PHY_INTERRUPT_CPM
//mii_link_interrupt(void *dev_id, struct pt_regs *regs)?
mii_link_interrupt(void *dev_id)
#else
mii_link_interrupt(int irq, void * dev_id)
#endif
{
	struct	net_device *dev = dev_id;
	struct fec_enet_private *fep = dev->priv;

#ifdef MDIO_DEBUG_LINK
	printk("  mii_link_interrupt\n");
#endif

	if (fep->phy) {
		/* restart or display status */
		mii_do_cmd(dev, fep->phy->ack_int);
	}
	return IRQ_RETVAL(IRQ_HANDLED);
}
#endif /* ifdef HAVE_PHY_INTERRUPT */

#ifdef CONFIG_PRxK
static int cnt_check = 0, phy_status = 0;
void check_fec_neg_complete (void)
{
	int old_status;
	struct	net_device *dev = NULL;
	struct fec_enet_private *fep;

	if (!dev) return;

	if ((fep = dev->priv) == NULL) return;

	if (fep->phy_id_done != 1) return;

	if (!fep->phy->poll_cnt) return;

	if (fep->phy->poll_cnt <= cnt_check++) {
		cnt_check = 0;
		mii_do_cmd(dev, fep->phy->ack_int);
	} else {
		old_status = phy_status;
		phy_status = fep->phy_status & (PHY_STAT_SPMASK | 
				PHY_STAT_LINK | PHY_STAT_ANC | PHY_STAT_FAULT);
		if (old_status != phy_status) {
			/* restart and display status */
			mii_relink(dev);
			mii_display_status(dev);
		}
	}
}
#endif

#if defined(DEBUG_FEC_INTR_RX) || defined(DEBUG_FEC_INTR_TX)

#if 0
unsigned long seq1 = 0;
unsigned long seq2 = 0;
unsigned long seqa = 0;
#endif

void dump_buf(unsigned char *buf, int pkt_len, int flag)
{
	int i;
#if 0
	unsigned long seq;

	if (buf[0] != 0x45 || buf[9] != 0x06)
		return;

	if (*(unsigned short *)&buf[20] != 20  && (unsigned short *)&buf[22] != 20)
		return;
 
	if (flag == 1) {
		seq = *(unsigned long *)((char *)&buf[28]);
		if (seq == seqa)
			printk("\nEth0T[%d] lost tx ack %x", pkt_len, seq);
		seqa = seq;
		return;
	}
	if (flag == 0) {
		seq = *(unsigned long *)((char *)&buf[24]);
		if (seq == seq1 || seq == seq2) {
			printk("\nEth0R[%d] retrans seq %x seq1 %x seq2 %x", pkt_len, seq, seq1, seq2);
			return;
		}
		if (seq < seq1 || seq < seq2) {
			printk("\nEth0R[%d] lost seq %x seq1 %x seq2 %x", pkt_len, seq, seq1, seq2);
		}
		seq1 = seq2;
		seq2 = seq;
		return;
	}
	seq = *(unsigned long *)((char *)&buf[24]);
	printk("\nEth0T[%d] rx error seq %x", pkt_len, seq);
#endif
	if (flag) {
		printk("\nEth0T[%d]:", pkt_len);
	} else {
		printk("\nEth0R[%d]:", pkt_len);
	}
	i = pkt_len > 80 ? 80 : pkt_len;
	for( ; i--; )
		printk("%02x", *buf++);
}
#endif

static int
fec_enet_open(struct net_device *dev)
{
	struct fec_enet_private *fep = dev->priv;
	bd_t *bd = (bd_t *)__res;

	/* I should reset the ring buffers here, but I don't yet know
	 * a simple way to do that.
	 */

#ifdef	CONFIG_USE_MDIO

#ifdef CONFIG_PRxK
	if (fep->phy) {
        if (bd->mii_operation == 0) {
			fep->phy->ix_start= 0;
		} else {
			fep->phy->ix_start= 1;
		}
		fep->phy->ix_conf = bd->mii_operation;
	}
#endif

	fep->sequence_done = 0;

	if (fep->phy) {
		fec_restart(dev, 0);	/* always start in half-duplex */
		mii_do_cmd(dev, ack_int1);
#ifdef CONFIG_PRxK
		mii_do_cmd(dev, fep->phy->config[fep->phy->ix_conf]);
#else
		mii_do_cmd(dev, fep->phy->config);
#endif
		mii_do_cmd(dev, phy_cmd_config);  /* display configuration */
		while(!fep->sequence_done)
			schedule();

#ifdef CONFIG_PRxK
		mii_do_cmd(dev, fep->phy->startup[fep->phy->ix_start]);
#else
		mii_do_cmd(dev, fep->phy->startup);
#endif
		netif_start_queue(dev);
		netif_carrier_off(dev); //[RK]Sep/09/05 - start OFF
		return 0;		/* Success */
	}
	return -ENODEV;		/* No PHY we understand */
#else	/* CONFIG_USE_MDIO */

	fec_restart(dev, 0);	/* always start in half-duplex */
	netif_start_queue(dev);
	return 0;	/* Success */
#endif	/* CONFIG_USE_MDIO */

}

static int
fec_enet_close(struct net_device *dev)
{
#ifdef	CONFIG_USE_MDIO
	struct fec_enet_private *fep = dev->priv;
#endif
	netif_stop_queue(dev);
	fec_stop(dev);
#ifdef	CONFIG_USE_MDIO
	if (fep->phy)
		mii_do_cmd(dev, fep->phy->shutdown);
#endif

	return 0;
}

static struct net_device_stats *fec_enet_get_stats(struct net_device *dev)
{
	struct fec_enet_private *fep = (struct fec_enet_private *)dev->priv;

	return &fep->stats;
}

/* Set or clear the multicast filter for this adaptor.
 * Skeleton taken from sunlance driver.
 * The CPM Ethernet implementation allows Multicast as well as individual
 * MAC address filtering.  Some of the drivers check to make sure it is
 * a group multicast address, and discard those that are not.  I guess I
 * will do the same for now, but just remove the test if you want
 * individual filtering as well (do the upper net layers want or support
 * this kind of feature?).
 */

#define HASH_BITS	6		/* #bits in hash */
#define CRC32_POLY	0xEDB88320

static void set_multicast_list(struct net_device *dev)
{
	struct	fec_enet_private *fep;
	volatile fec_t *ep;
	struct dev_mc_list *dmi;
	unsigned int i, j, bit, data, crc;
	unsigned char hash;

	fep = (struct fec_enet_private *)dev->priv;
	ep = &(((immap_t *)IMAP_ADDR)->im_cpm.cp_fec);

	if (dev->flags&IFF_PROMISC) {

		/* Log any net taps. */
		printk("%s: Promiscuous mode enabled.\n", dev->name);
		ep->fec_r_cntrl |= FEC_RCNTRL_PROM;

	} else {

		ep->fec_r_cntrl &= ~FEC_RCNTRL_PROM;

		if (dev->flags & IFF_ALLMULTI) {
			/* Catch all multicast addresses, so set the
			 * filter to all 1's.
			 */
			ep->fec_hash_table_high = 0xffffffff;
			ep->fec_hash_table_low = 0xffffffff;
		}

		else {
			/* Clear filter and add the addresses in hash register.
			*/
			ep->fec_hash_table_high = 0;
			ep->fec_hash_table_low = 0;

			dmi = dev->mc_list;

			for (j = 0; j < dev->mc_count; j++, dmi = dmi->next)
			{
				/* Only support group multicast for now.
				*/
				if (!(dmi->dmi_addr[0] & 1))
					continue;

				/* calculate crc32 value of mac address
				*/
				crc = 0xffffffff;

				for (i = 0; i < dmi->dmi_addrlen; i++)
				{
					data = dmi->dmi_addr[i];
					for (bit = 0; bit < 8; bit++, data >>= 1)
					{
						crc = (crc >> 1) ^
						(((crc ^ data) & 1) ? CRC32_POLY : 0);
					}
				}

				/* only upper 6 bits (HASH_BITS) are used
				   which point to specific bit in he hash registers
				*/
				hash = (crc >> (32 - HASH_BITS)) & 0x3f;

				if (hash > 31)
					ep->fec_hash_table_high |= 1 << (hash - 32);
				else
					ep->fec_hash_table_low |= 1 << hash;
			}
		}
	}
}

static int fec_set_mac_address(struct net_device *dev, void *p)
{
	struct sockaddr *addr=p;
	volatile        fec_t   *fecp;

//	if(dev->start)
//		return -EBUSY;
	memcpy(dev->dev_addr, addr->sa_data,dev->addr_len);

	fecp = (fec_t *)dev->base_addr;

	/* Set station address. */
	fecp->fec_addr_low =    (dev->dev_addr[0] << 24) |
				(dev->dev_addr[1] << 16) |
				(dev->dev_addr[2] <<  8) |
				dev->dev_addr[3];
	fecp->fec_addr_high = (dev->dev_addr[4] << 8) | dev->dev_addr[5];

	return(0);
}

#ifdef CONFIG_PRxK
static int fec_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct fec_enet_private *fep = dev->priv;
	u16 *data = (u16*)&ifr->ifr_ifru;
	
	if (!fep) {
		return 0;
	}
	// SNMP uses the SIOCDEVPRIVATE to get informations about the interface
	switch (cmd) {
		case SIOCDEVPRIVATE:	// Get address of MII PHY in use
			data[0] = fep->phy_id;
			return(0);

		case SIOCDEVPRIVATE+1:	// read MII PHY register
			switch (data[1] & 0x1f) {
				case 0 : // basic mode control register
					if (fep->phy_status & (PHY_STAT_100FDX | PHY_STAT_100HDX)) {
						data[3] = 0x2000;
					} else {
						data[3] = 0x00;
					}
					break;
				case 1: // basic mode status register
					data[3] = 0x0004;
					break;
				default :
					data[3] = 0;
					break;
			}
			return(0);
		
		case SIOCDEVPRIVATE+2: // write MII PHY register
			return(0);

		case SIOCDEVPRIVATE+3: // write the MII operation
			((bd_t *)__res)->mii_operation = ifr->ifr_ifru.ifru_ivalue;

			if (ifr->ifr_ifru.ifru_flags)
				return(0);
			return(0);

		case SIOCDEVPRIVATE+4: // write the MAX Interrupt Work
			((bd_t *)__res)->max_int_work = ifr->ifr_ifru.ifru_ivalue;
			MAX_INTERRUPT_WORK = ifr->ifr_ifru.ifru_ivalue;
			return(0);
		
		case SIOCDEVPRIVATE+5: // write the max receive bytes 
			((bd_t *)__res)->ibytesec = ifr->ifr_ifru.ifru_ivalue;
			ibytesec = ifr->ifr_ifru.ifru_ivalue;
			return(0);
	}
	return -EINVAL;
}
#endif
			 
/* Initialize the FEC Ethernet on 860T.
 */
int __init fec_enet_init(void)
{
	struct net_device *dev;
	struct fec_enet_private *fep;
	int i, j;
	unsigned char	*iap;
	void		*mem_addr;
	volatile	cbd_t	*bdp;
	cbd_t		*cbd_base;
	volatile	immap_t	*immap;
	volatile	fec_t	*fecp;
	bd_t		*bd;
	int	err;
	dma_addr_t     physaddr;
#ifdef CONFIG_SCC_ENET
	unsigned char	tmpaddr[6];
#endif

	immap = (immap_t *)IMAP_ADDR;	/* pointer to internal registers */

	bd = (bd_t *)__res;

#ifdef CONFIG_PRxK
	{
		volatile sysconf8xx_t *psiu = &(immap->im_siu_conf);
		uint siumcr = psiu->sc_siumcr;
		siumcr |= 0x00030240; //set bits FRC,DLK,AEME,GB5E (see: cyboot/sw/boot/reset/initacs.s:PCMCIA_EN)
		psiu->sc_siumcr = siumcr;
	}
#endif

	dev = alloc_etherdev(sizeof(*fep));
	if (!dev)
		return -ENOMEM;

	/* Allocate some private information.
	*/
	fep = dev->priv;
	if (fep == NULL)
		return -ENOMEM;

	__clear_user(fep,sizeof(*fep));

	/* Create an Ethernet device instance.
	*/
	fecp = &(immap->im_cpm.cp_fec);

	/* Whack a reset.  We should wait for this.
	*/
	fecp->fec_ecntrl = FEC_ECNTRL_PINMUX | FEC_ECNTRL_RESET;
	for (i = 0;
	     (fecp->fec_ecntrl & FEC_ECNTRL_RESET) && (i < FEC_RESET_DELAY);
	     ++i) {
		udelay(1);
	}
	if (i == FEC_RESET_DELAY) {
		printk ("FEC Reset timeout!\n");
	}

	/* Set the Ethernet address.  If using multiple Enets on the 8xx,
	 * this needs some work to get unique addresses.
	 */
	iap = bd->bi_enetaddr;

#ifdef CONFIG_SCC_ENET
	/*
         * If a board has Ethernet configured both on a SCC and the
         * FEC, it needs (at least) 2 MAC addresses (we know that Sun
         * disagrees, but anyway). For the FEC port, we create
         * another address by setting one of the address bits above
         * something that would have (up to now) been allocated.
	 */
	for (i=0; i<6; i++)
		tmpaddr[i] = *iap++;
	tmpaddr[3] |= 0x80;
	iap = tmpaddr;
#endif

	for (i=0; i<6; i++) {
		dev->dev_addr[i] = *iap++;
	}

	/* Allocate memory for buffer descriptors.
	*/
	if (((RX_RING_SIZE + TX_RING_SIZE) * sizeof(cbd_t)) > PAGE_SIZE) {
		printk("FEC init error.  Need more space.\n");
		printk("FEC initialization failed.\n");
		return 1;
	}
	mem_addr = dma_alloc_coherent(NULL, PAGE_SIZE, &physaddr, GFP_KERNEL);
	cbd_base = (cbd_t *) mem_addr;

	/* Set receive and transmit descriptor base.
	*/
	fep->rx_bd_base = cbd_base;
	fep->tx_bd_base = cbd_base + RX_RING_SIZE;

	fep->skb_cur = fep->skb_dirty = 0;


	mem_addr = dma_alloc_coherent(NULL, FEC_ENET_RX_PAGES*PAGE_SIZE, &physaddr, GFP_KERNEL);
	fep->va_rx_base = fep->va_rx_cur = mem_addr;

	/* Initialize the receive buffer descriptors.
	*/
	bdp = fep->rx_bd_base;
	for (i=0; i<FEC_ENET_RX_PAGES; i++) {

		/* Initialize the BD for every fragment in the page.
		*/
		for (j=0; j<FEC_ENET_RX_FRPPG; j++) {
			bdp->cbd_sc = BD_ENET_RX_EMPTY;
			bdp->cbd_bufaddr = physaddr;
			physaddr += FEC_ENET_RX_FRSIZE;
			bdp++;
		}
	}

	/* Set the last buffer to wrap.
	*/
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

#ifdef CONFIG_FEC_PACKETHOOK
	fep->ph_lock = 0;
	fep->ph_rxhandler = fep->ph_txhandler = NULL;
	fep->ph_proto = 0;
	fep->ph_regaddr = NULL;
	fep->ph_priv = NULL;
#endif
#ifdef CONFIG_PRxK
	//[GB]May/06/05  Ethernet Receive Rate Limit
	fep->ibr = 0; /* initial count of received bytes */
	fep->t1 = jiffies;
	fep->tlog = jiffies; /* just to put anything valid here */
#endif
	/* Install our interrupt handler.
	*/
	if (request_irq(FEC_INTERRUPT, fec_enet_interrupt, 0, "fec", dev) != 0)
		panic("Could not allocate FEC IRQ!");

#ifdef CONFIG_FADS
	/* Enable the LXT970 PHY. */
	{
		volatile uint *bcsr4 = (volatile uint *) BCSR4;

		*bcsr4 = (*bcsr4 & ~(BCSR4_FETH_EN | BCSR4_FETHCFG1))
			| (BCSR4_FETHCFG0 | BCSR4_FETHFDE | BCSR4_FETHRST);
		/* reset the PHY */
		*bcsr4 &= ~BCSR4_FETHRST;
		udelay(10);
		*bcsr4 |= BCSR4_FETHRST;
		udelay(1000);
	}
#endif	/* ifdef CONFIG_FADS */

#ifdef CONFIG_RPXCLASSIC
	/* Make LEDS reflect Link status.
	*/
	*((volatile uint *) RPX_CSR_ADDR) &= ~(BCSR2_FETHLEDMODE | BCSR2_MIIRST
					     | BCSR2_MIIPWRDWN);
	/* route PHY interrupt to PC15 for compatibility with older CLLFs
	*/
	*((volatile uint *) RPX_CSR_ADDR) |= BCSR2_FIRQPC15;
	/* Enable external LXT971 PHY on the CLLF BW31.  This is
	 * harmless on older CLLFs.
	*/
	*((volatile uint *) RPX_CSR_ADDR) |= BCSR2_MIIRST;
	udelay(1000);
	*((volatile uint *) RPX_CSR_ADDR) |= BCSR2_MIIPWRDWN;
	udelay(1000);
#endif	/* ifdef CONFIG_RPXCLASSIC */

#ifdef CONFIG_PRxK
	/* reset 100BaseT through PC4 */
	immap->im_ioport.iop_pcdir |= 0x0800;
	immap->im_ioport.iop_pcpar &= ~(0x0800);
	immap->im_ioport.iop_pcdat &= ~(0x0800);
	udelay(10);
	immap->im_ioport.iop_pcdat |= 0x0800;
	udelay(1000);
#endif
	
	dev->base_addr = (unsigned long)fecp;
	dev->priv = fep;

	/* The FEC Ethernet specific entries in the device structure. */
	dev->open = fec_enet_open;
	dev->hard_start_xmit = fec_enet_start_xmit;
	dev->tx_timeout = fec_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;
	dev->stop = fec_enet_close;
	dev->get_stats = fec_enet_get_stats;
	dev->set_multicast_list = set_multicast_list;
	dev->set_mac_address = fec_set_mac_address;
#ifdef CONFIG_PRxK
	dev->do_ioctl = fec_do_ioctl;
#endif
#ifdef	CONFIG_USE_MDIO
	for (i=0; i<NMII-1; i++)
		mii_cmds[i].mii_next = &mii_cmds[i+1];
	mii_free = mii_cmds;
#endif	/* CONFIG_USE_MDIO */

	/* Configure all of port D for MII.
	*/
	immap->im_ioport.iop_pdpar = 0x1fff;

	/* Bits moved from Rev. D onward.
	*/
	if ((mfspr(SPRN_IMMR) & 0xffff) < 0x0501)
		immap->im_ioport.iop_pddir = 0x1c58;	/* Pre rev. D */
	else
		immap->im_ioport.iop_pddir = 0x1fff;	/* Rev. D and later */

#ifdef	CONFIG_USE_MDIO
	/* Set MII speed to 2.5 MHz
	*/
	fecp->fec_mii_speed = fep->phy_speed =
		(( (bd->bi_intfreq + 500000) / 2500000 / 2 ) & 0x3F ) << 1;
#else
	fecp->fec_mii_speed = 0;	/* turn off MDIO */
#endif	/* CONFIG_USE_MDIO */

	err = register_netdev(dev);
	if (err) {
		free_netdev(dev);
		return err;
	}

	printk ("%s: FEC ENET Version 0.2, FEC irq %d"
#ifdef PHY_INTERRUPT
		", MII irq %d"
#endif
		", addr ",
		dev->name, FEC_INTERRUPT
#ifdef PHY_INTERRUPT
		, PHY_INTERRUPT
#endif
	);
	for (i=0; i<6; i++)
		printk("%02x%c", dev->dev_addr[i], (i==5) ? '\n' : ':');

#ifdef PHY_INTERRUPT_CPM
	{
		/* Enable the interrupt input in Port C corresponding to the
		 * CPM interrupt vector used for the PHY.
		 */
		const ushort cpmvec_to_portc_mask[CPMVEC_NR] =
			{ 0x0000, 0x0800, 0x0400, 0x0000,
			  0x0000, 0x0000, 0x0200, 0x0000,
			  0x0000, 0x0100, 0x0080, 0x0040,
			  0x0000, 0x0000, 0x0020, 0x0010,
			  0x0000, 0x0000, 0x0000, 0x0000,
			  0x0000, 0x0000, 0x0000, 0x0008,
			  0x0004, 0x0000, 0x0002, 0x0000,
			  0x0000, 0x0000, 0x0000, 0x0001 };
		ushort portc_mask =
			cpmvec_to_portc_mask[PHY_INTERRUPT_CPM & 0x1f];

		immap->im_ioport.iop_pcpar &= ~portc_mask;
		immap->im_ioport.iop_pcdir &= ~portc_mask;
		immap->im_ioport.iop_pcso  &= ~portc_mask;
		immap->im_ioport.iop_pcint |=  portc_mask;
		cpm_install_handler(PHY_INTERRUPT_CPM, mii_link_interrupt, dev);
	}
#endif	/* ifdef PHY_INTERRUPT_CPM */

#ifdef PHY_INTERRUPT
	((immap_t *)IMAP_ADDR)->im_siu_conf.sc_siel |=
		(0x80000000 >> PHY_INTERRUPT);

	if (request_irq(PHY_INTERRUPT, mii_link_interrupt, 0, "mii", dev) != 0)
		panic("Could not allocate MII IRQ!");
#endif	/* ifdef PHY_INTERRUPT */

#ifdef	CONFIG_USE_MDIO
	/* enable the MDIO interface */
	fec_enable_mdio(dev);

	/* Queue up command to detect the PHY and initialize the
	 * remainder of the interface.
	 */
	fep->phy_id_done = 0;
	fep->phy_addr = 0;
	mii_queue(dev, mk_mii_read(MII_PHYSID1), mii_discover_phy);
#endif	/* CONFIG_USE_MDIO */

	return 0;
}

module_init(fec_enet_init);


/* This function is called to start or restart the FEC during a link
 * change.  This only happens when switching between half and full
 * duplex.
 */
static void
fec_restart(struct net_device *dev, int duplex)
{
	struct fec_enet_private *fep;
	int i;
	volatile	cbd_t	*bdp;
	volatile	immap_t	*immap;
	volatile	fec_t	*fecp;
	bd_t            *bd = (bd_t *)__res;
	
	/* stop any transmissions in progress */
	fec_stop(dev);

	immap = (immap_t *)IMAP_ADDR;	/* pointer to internal registers */

	fecp = &(immap->im_cpm.cp_fec);

	fep = dev->priv;

	/* Whack a reset.  We should wait for this.
	*/
	fecp->fec_ecntrl = FEC_ECNTRL_PINMUX | FEC_ECNTRL_RESET;
	for (i = 0;
	     (fecp->fec_ecntrl & FEC_ECNTRL_RESET) && (i < FEC_RESET_DELAY);
	     ++i) {
		udelay(1);
	}
	if (i == FEC_RESET_DELAY) {
		printk ("FEC Reset timeout!\n");
	}
	for (i=0; i<NMII-1; i++)
		mii_cmds[i].mii_next = &mii_cmds[i+1];
		mii_free = mii_cmds;
	mii_head = NULL;
	
	/* Set station address.
	*/
	fecp->fec_addr_low =    (dev->dev_addr[0] << 24) |
				(dev->dev_addr[1] << 16) |
				(dev->dev_addr[2] <<  8) |
				 dev->dev_addr[3];
	fecp->fec_addr_high = (dev->dev_addr[4] << 8) | dev->dev_addr[5];

	/* Reset all multicast.
	*/
	fecp->fec_hash_table_high = 0;
	fecp->fec_hash_table_low  = 0;

	/* Set maximum receive buffer size.
	*/
	fecp->fec_r_buff_size = PKT_MAXBLR_SIZE;
	fecp->fec_r_hash = PKT_MAXBUF_SIZE;

	/* Set receive and transmit descriptor base.
	*/
	fecp->fec_r_des_start = iopa((uint)(fep->rx_bd_base));
	fecp->fec_x_des_start = iopa((uint)(fep->tx_bd_base));

	fep->dirty_tx = fep->cur_tx = fep->tx_bd_base;
	fep->tx_free = TX_RING_SIZE;
	fep->cur_rx = fep->rx_bd_base;
	fep->va_rx_cur = fep->va_rx_base;/*added 2.6*/
	fep->skb_cur = fep->skb_dirty = 0;

	/* Initialize the receive buffer descriptors.
	*/
	bdp = fep->rx_bd_base;
	for (i=0; i<RX_RING_SIZE; i++) {

		/* Initialize the BD for every fragment in the page.
		*/
		bdp->cbd_sc = BD_ENET_RX_EMPTY;
		bdp++;
	}

	/* Set the last buffer to wrap.
	*/
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

	/* ...and the same for transmit.
	*/
	bdp = fep->tx_bd_base;
	for (i=0; i<TX_RING_SIZE; i++) {

		/* Initialize the BD for every fragment in the page.
		*/
		bdp->cbd_sc = 0;
		bdp->cbd_bufaddr = 0;
		bdp++;
	}

	/* Set the last buffer to wrap.
	*/
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

	MAX_INTERRUPT_WORK = bd->max_int_work;
#ifdef CONFIG_PRxK	
	ibytesec = bd->ibytesec; //[GB]May/06/05  Ethernet Receive Rate Limit
#endif

	/* Enable MII mode.
	*/
	if (duplex) {
		fecp->fec_r_cntrl = FEC_RCNTRL_MII_MODE;	/* MII enable */
		fecp->fec_x_cntrl = FEC_TCNTRL_FDEN;		/* FD enable */
	}
	else {
		fecp->fec_r_cntrl = FEC_RCNTRL_MII_MODE | FEC_RCNTRL_DRT;
		fecp->fec_x_cntrl = 0;
	}

	/* Enable big endian and don't care about SDMA FC.
	*/
	fecp->fec_fun_code = 0x78000000;

#ifdef	CONFIG_USE_MDIO
	/* Set MII speed.
	*/
	fecp->fec_mii_speed = fep->phy_speed;
#endif	/* CONFIG_USE_MDIO */

	/* Clear any outstanding interrupt.
	*/
	fecp->fec_ievent = 0xffc0;

	fecp->fec_ivec = (FEC_INTERRUPT/2) << 29;

	/* Enable interrupts we wish to service.
	*/
	fecp->fec_imask = ( FEC_ENET_TXF | FEC_ENET_TXB |
			    FEC_ENET_RXF | FEC_ENET_RXB | FEC_ENET_MII );

	/* And last, enable the transmit and receive processing.
	*/
	fecp->fec_ecntrl = FEC_ECNTRL_PINMUX | FEC_ECNTRL_ETHER_EN;
	fecp->fec_r_des_active = 0x01000000;
#ifdef CONFIG_PRxK
	immap->im_ioport.iop_pcdat &= ~(0x0800);
	udelay(30000); /* for 2.4 it was udelay(300000); */
	immap->im_ioport.iop_pcdat |= 0x0800;
	udelay(1000);
#endif
}

static void
fec_stop(struct net_device *dev)
{
	volatile	immap_t	*immap;
	volatile	fec_t	*fecp;
	struct fec_enet_private *fep;
	int i;

	immap = (immap_t *)IMAP_ADDR;	/* pointer to internal registers */

	fecp = &(immap->im_cpm.cp_fec);

	if ((fecp->fec_ecntrl & FEC_ECNTRL_ETHER_EN) == 0)
		return;	/* already down */

	fep = dev->priv;

	fecp->fec_x_cntrl = 0x01;	/* Graceful transmit stop */

	for (i = 0;
	     ((fecp->fec_ievent & 0x10000000) == 0) && (i < FEC_RESET_DELAY);
	     ++i) {
		udelay(1);
	}
	if (i == FEC_RESET_DELAY) {
		printk ("FEC timeout on graceful transmit stop\n");
	}

	/* Disable FEC
	*/
	fecp->fec_ecntrl &= ~(FEC_ECNTRL_ETHER_EN);

	/* Reset SKB transmit buffers.
	*/
	fep->skb_cur = fep->skb_dirty = 0;
	for (i=0; i<=TX_RING_MOD_MASK; i++) {
		if (fep->tx_skbuff[i] != NULL) {
			dev_kfree_skb(fep->tx_skbuff[i]);
			fep->tx_skbuff[i] = NULL;
		}
	}
}

#ifdef	CONFIG_USE_MDIO
static void
fec_enable_mdio(struct net_device *dev)
{
	volatile	immap_t	*immap;
	volatile	fec_t	*fecp;
	struct fec_enet_private *fep;

	immap = (immap_t *)IMAP_ADDR;	/* pointer to internal registers */

	fecp = &(immap->im_cpm.cp_fec);

	fep = dev->priv;

	/* Clear outstanding MII command interrupts.
	*/
	fecp->fec_ievent = FEC_ENET_MII;

	/* Enable MII command finished interrupt
	*/
	fecp->fec_ivec = (FEC_INTERRUPT/2) << 29;
	fecp->fec_imask |= FEC_ENET_MII;

	/* Set MII speed.
	*/
	fecp->fec_mii_speed = fep->phy_speed;

	/* Enable FEC pin MUX
	*/
	fecp->fec_ecntrl |= FEC_ECNTRL_PINMUX;

	fecp->fec_r_cntrl = FEC_RCNTRL_MII_MODE;	/* MII enable */
}
#endif	/* CONFIG_USE_MDIO */
