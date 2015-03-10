/*----------------------------------------------------------------------------
 *      Copyright (C) Cyclades Corporation, 1998.
 *      All rights reserved
 *      PR-3000 TS
 *
 *      This file:      cy_wdt.c
 *      Description:    Routines to handle WDT and CPU led.
 *---------------------------------------------------------------------------
 *---------------------------------------------------------------------------
 *      Change History
 *---------------------------------------------------------------------------
 *      Sep/28/2000     V 1.0.0    Edson Ap. Seabra - Initial Implementation
 *--------------------------------------------------------------------------*/

//#include <linux/config.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
//#include <linux/malloc.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/system.h>

#include <asm/mpc8xx.h>
#include <asm/8xx_immap.h>

#include <asm/cpld.h>

#define IS_BOARD_KVMANALOG	(bd->hw_info.board_type == BOARD_KVM16 || \
	bd->hw_info.board_type == BOARD_KVM32)

#define IS_BOARD_KVMNET	(bd->hw_info.board_type == BOARD_KVMNET16 || \
	bd->hw_info.board_type == BOARD_KVMNET32)

#define IS_BOARD_KVM	(IS_BOARD_KVMANALOG || IS_BOARD_KVMNET)

#define IS_BOARD_ACS	(bd->hw_info.board_type == BOARD_TS1100 || \
			bd->hw_info.board_type == BOARD_TS2100 || \
			bd->hw_info.board_type == BOARD_ACS48 || \
			bd->hw_info.board_type == BOARD_ACS8 || \
			bd->hw_info.board_type == BOARD_ACS4 || \
			bd->hw_info.board_type == BOARD_ACS1 || \
			bd->hw_info.board_type == BOARD_AVCS1 || \
			bd->hw_info.board_type == BOARD_AVCS4 || \
			bd->hw_info.board_type == BOARD_AVCS8 || \
			bd->hw_info.board_type == BOARD_AVCS16 || \
			bd->hw_info.board_type == BOARD_AVCS32 || \
			bd->hw_info.board_type == BOARD_AVCS48 || \
			bd->hw_info.board_type == BOARD_ACS5001 || \
			bd->hw_info.board_type == BOARD_ACS5004 || \
			bd->hw_info.board_type == BOARD_ACS5008 || \
			bd->hw_info.board_type == BOARD_ACS5016 || \
			bd->hw_info.board_type == BOARD_ACS5032 || \
			bd->hw_info.board_type == BOARD_ACS5048)

static int wdt_is_open=0;
static int led_is_open=0;

static long long wdt_llseek(struct file *file, long long offset, int origin)
{
	return -ESPIPE;
}

static ssize_t wdt_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	struct inode *inode;
	bd_t *bd = (bd_t *)__res;

	inode = file->f_dentry->d_inode;

	switch(MINOR(inode->i_rdev))
	{
		case WATCHDOG_MINOR:
			if(!wdt_is_open) {
				return -ESPIPE;
			} else {
				volatile sysconf8xx_t *psiu = &(((immap_t *)IMAP_ADDR)->im_siu_conf);
				psiu->sc_swsr = 0x556c;
				psiu->sc_swsr = 0xaa39;
			}
			break;
		case TEMP_MINOR:
		//	check_fec_neg_complete();
			if(!led_is_open) {
				return -ESPIPE;
			}
#ifdef CONFIG_NL1K
			if (*buf) {
				fpga->fpga_led |= 0x01;
			} else {
				fpga->fpga_led &= ~0x01;
			}
#elif defined(CONFIG_TS1H)
			{
				volatile cpm8xx_t *pcpm = &(((immap_t *)IMAP_ADDR)->im_cpm);
				if (*buf) {
		 			pcpm->cp_pbdat &= ~(0x0001); 			
				} else {
		 			pcpm->cp_pbdat |= 0x0001; 			
				}
			}
#else

			if (IS_BOARD_ACS || IS_BOARD_KVM) {
				if (*buf) {
					cpld->cpld_misc |=  0x01;
				} else {
					cpld->cpld_misc &= ~0x01;
				}
			} else {
				volatile cpm8xx_t *pcpm = 
					&(((immap_t *)IMAP_ADDR)->im_cpm);
				if (*buf) {
		 			pcpm->cp_pbdat &= ~(0x0001); 			
				} else {
		 			pcpm->cp_pbdat |= 0x0001; 			
				}
			}
#endif
			break;
		default:
			return -ENODEV;
	}
	return 1;
}

/*
 *	Read reports the temperature in degrees Fahrenheit.
 */
 
static ssize_t wdt_read(struct file *file, char *buf, size_t count, loff_t *ptr)
{
        struct inode *inode;

        inode = file->f_dentry->d_inode;

	if (MINOR(inode->i_rdev) != TEMP_MINOR) {
		return -ESPIPE;
	}
	*buf = ((bd_t *)__res)->hw_info.error_status;
	return 1;
}

extern void m8xx_del_timer(void);

static int wdt_open(struct inode *inode, struct file *file)
{
	bd_t *bd = (bd_t *)__res;
	switch(MINOR(inode->i_rdev))
	{
		case WATCHDOG_MINOR:
			if(wdt_is_open)
				return -EBUSY;
			wdt_is_open=1;
			/* watchdog now handled by cy_wdt_led, stop timer */
                        m8xx_del_timer();
        	((bd_t *)__res)->wdt |= 2;
			break;
		case TEMP_MINOR:
#ifdef CONFIG_TS1H
			if (led_is_open)
				return -EBUSY;
			led_is_open = 1;
#else
			if (!led_is_open) {
				if (IS_BOARD_KVMNET) {
					/* Fan speed threshold (2000 rpm) */
					cpld->intr_status4 = 0x02; 
					/* Enable Fan 1 & Fan 2 monitoring */
					cpld->fpga_led_ctl |= (0x04|0x02); 
					/* Pause for 1/50 second to give the
					   FPGA time to interpret that last setting
					   so that we avoid a "blip" from the
					   buzzer */
					set_current_state(TASK_UNINTERRUPTIBLE);
					schedule_timeout(HZ/50);
					/* Enable alarm on fan failure */
					cpld->fpga_cfst |= 0x08;
				} else if (IS_BOARD_KVMANALOG) {
					cpld->fpga_led_ctl |= 2; 
				}
printk(KERN_DEBUG "Register %04x value %02x\n", (int)&(cpld->fpga_cfst), 
	cpld->fpga_cfst);
printk(KERN_DEBUG "Register %04x value %02x\n", (int)&(cpld->intr_status4),
	cpld->intr_status4);
printk(KERN_DEBUG "Register %04x value %02x\n", (int)&(cpld->cpld_misc),
	cpld->cpld_misc);
printk(KERN_DEBUG "Register %04x value %02x\n", (int)&(cpld->fpga_led_ctl), 
	cpld->fpga_led_ctl);
			}
			led_is_open++;
#endif
			break;
		default:
			return -ENODEV;
	}
	return 0;
}

static int wdt_release(struct inode *inode, struct file *file)
{
	bd_t *bd = (bd_t *)__res;
	switch(MINOR(inode->i_rdev))
	{
		case WATCHDOG_MINOR:
			if(!wdt_is_open)
				return -ESPIPE;
			wdt_is_open=0;
        	((bd_t *)__res)->wdt &= 1;
			break;
		case TEMP_MINOR:
			if(!led_is_open) 
				return -ESPIPE;
#ifdef CONFIG_TS1H
			led_is_open = 0;
#else
			led_is_open--;
			if(!led_is_open) {
				if (IS_BOARD_KVM) {
					cpld->fpga_led_ctl &= ~6;
					cpld->fpga_cfst &= ~0x0c;
printk(KERN_DEBUG "Register %04x value %02x\n", (int)&(cpld->fpga_cfst), 
	cpld->fpga_cfst);
printk(KERN_DEBUG "Register %04x value %02x\n", (int)&(cpld->fpga_led_ctl), 
	cpld->fpga_led_ctl);
				}
			}
#endif
			break;
		default:
			return -ENODEV;
	}
	return 0;
}

static int wdt_ioctl(struct inode * inode, struct file * file,
          unsigned int cmd, unsigned long arg)
{
	bd_t *bd = (bd_t *)__res;
	int __user *p = (int __user *) arg;

	if (MINOR(inode->i_rdev) != TEMP_MINOR) {
		return -ESPIPE;
	}
#ifndef CONFIG_TS1H
	switch (cmd) {
	case 0: /* return true if dual power suply */
		if (IS_BOARD_ACS) {
			return(put_user(bd->hw_info.power_supply, p));
		}
		return(put_user(0, p));

	case 1: /* return the status of both power supplies */
		if (IS_BOARD_ACS && bd->hw_info.power_supply) {
			return(put_user(((cpld->cpld_misc & 0x30) >> 4), p));
		}
		return(put_user(0, p));

	case 2:
		if (IS_BOARD_ACS) {
			cpld->cpld_misc &=  ~0x08;
			return(0);
		}
		break;

	case 3:
		if (IS_BOARD_ACS) {
			cpld->cpld_misc ^=  0x08;
			return(0);
		}
		break;
		
	case 4:	/* return first fan speed */
		if (IS_BOARD_KVM && cpld->fpga_id >= 0x43) {
//printk(KERN_DEBUG "Register %04x value %02x\n", (int)&(cpld->intr_status3), 
//			cpld->intr_status3);
			return put_user(cpld->intr_status3 % 0x10, p);
		}
		return(put_user(0, p));
		
	case 5:	/* return second fan speed */
		if (IS_BOARD_KVM && cpld->fpga_id >= 0x43) {
			return put_user(cpld->intr_status3 / 0x10, p);
		}
		return(put_user(0, p));
		
	default:
		return -EINVAL;
		break;
	}
#endif

	return(0);
}

/*
 *	Kernel Interfaces
 */
 
 
static struct file_operations wdt_fops = {
	llseek:wdt_llseek,
	read:wdt_read,
	write:wdt_write,
	ioctl:wdt_ioctl,
	open:wdt_open,
	release:wdt_release
};

static struct miscdevice wdt_miscdev=
{
	WATCHDOG_MINOR,
	"PPC_wdt",
	&wdt_fops
};

static struct miscdevice led_miscdev=
{
	TEMP_MINOR,
	"CPU_led",
	&wdt_fops
};

int __init cy_wdt_init(void)
{
	bd_t *bd = (bd_t *)__res;

#ifdef CONFIG_NL1K
	fpga->fpga_led &= ~0x01;
#elif defined(CONFIG_TS1H)
	{
		volatile cpm8xx_t *pcpm = &(((immap_t *)IMAP_ADDR)->im_cpm); 
		pcpm->cp_pbpar &= ~(0x0001);
		pcpm->cp_pbdir |= 0x0001;
		pcpm->cp_pbdat |= 0x0001;
	}
#else
	if (IS_BOARD_ACS || IS_BOARD_KVM) {
		cpld->cpld_misc &= ~0x01;
	} else {
		volatile cpm8xx_t *pcpm = &(((immap_t *)IMAP_ADDR)->im_cpm); 
		pcpm->cp_pbpar &= ~(0x0001);
		pcpm->cp_pbdir |= 0x0001;
		pcpm->cp_pbdat |= 0x0001;
	}
	if (IS_BOARD_KVM) {
		cpld->fpga_cfst &= ~0x0c;
printk("Register %04x value %02x\n", (int)&(cpld->fpga_cfst), cpld->fpga_cfst);
		cpld->fpga_led_ctl &= ~6;
printk("Register %04x value %02x\n", (int)&(cpld->fpga_led_ctl), cpld->fpga_led_ctl);
	}
#endif

	misc_register(&wdt_miscdev);
	misc_register(&led_miscdev);
	return 0;
}

module_init(cy_wdt_init);


