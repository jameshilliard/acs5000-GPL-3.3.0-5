/* -*- linux-c -*- */
/*****************************************************************
 * File: cy_kvmip_led.h
 *
 * Copyright (C) 2005 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Description: Header file for Front-Panel LEDs device driver (KVM/net Plus)
 *
 * Date: 2/2005
 * Author: John Moghaddas <john.moghaddas@cyclades.com>
 * 
 ****************************************************************/

#undef PDEBUG
#ifdef IPLED_DEBUG
#  ifdef __KERNEL__
      /* Debugging is on and we are in kernel space */
#     define PDEBUG(fmt, args...) printk(KERN_DEBUG "kvmip_led: " fmt, ## args)
#  else
      /* Debugging is on and we are in user space */
#     define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...)   /* no debugging: nothing */
#endif

#define CYKVMIPLED_MAJOR	10		/* Major number */
#define CYKVMIPLED_MINOR	151		/* Minor number */

#define IP_LED_MASK 0xf  
#define IP1_LED	    0x1
#define IP2_LED	    0x2
#define IP3_LED	    0x4
#define IP4_LED	    0x8

#define CYKVMIPLED_IOC_MAGIC	CYKVMIPLED_MAJOR

#define CYKVMIPLEDGETSTAT	_IOR(CYKVMIPLED_IOC_MAGIC, 1, int)
#define CYKVMIPLEDSETON		_IOW(CYKVMIPLED_IOC_MAGIC, 2, int)
#define CYKVMIPLEDSETOFF	_IOW(CYKVMIPLED_IOC_MAGIC, 3, int)
#define CYKVMIPLEDSETLEDS	_IOW(CYKVMIPLED_IOC_MAGIC, 4, int)

/* setBit: return value with bit n set to v (0 or 1) */
unsigned int setBit(unsigned int word, int n, unsigned int v);
