/* -*- linux-c -*- */
/*****************************************************************
 * File: cy_apctl.h
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
 * Description: Header file apctl
 *
 * Date: 3/30/2005
 * Author: Erik Lotspeich <erik.lotspeich@cyclades.com>
 * 
 ****************************************************************/

#undef PDEBUG
#ifdef APCTL_DEBUG
#  ifdef __KERNEL__
      /* Debugging is on and we are in kernel space */
#     define PDEBUG(fmt, args...) printk(KERN_DEBUG "apctl: " fmt, ## args)
#  else
      /* Debugging is on and we are in user space */
#     define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...)   /* no debugging: nothing */
#endif

struct vidqual_set {
	int osd_num; /* OSD chip number (1 or 2) */
	int enable;  /* Enable? (0 or 1) */
};

#define CYAPCTL_MAJOR	10		/* Major number */
#define CYAPCTL_MINOR	240		/* Minor number */

#define CYAPCTL_IOC_MAGIC	CYAPCTL_MAJOR

#define CYAPCTLSETFANTHRESH	_IOW(CYAPCTL_IOC_MAGIC, 1, int)
#define CYAPCTLSETFANMON	_IOW(CYAPCTL_IOC_MAGIC, 2, int)
#define CYAPCTLSETALRM	        _IOW(CYAPCTL_IOC_MAGIC, 3, int)
#define CYAPCTLGETALRM	        _IOR(CYAPCTL_IOC_MAGIC, 4, int)
#define CYAPCTLKVMIPRESET       _IOW(CYAPCTL_IOC_MAGIC, 5, int)
#define CYAPCTLGETFANSPD        _IOR(CYAPCTL_IOC_MAGIC, 6, int)
#define CYAPCTLSETVIDQUAL       _IOW(CYAPCTL_IOC_MAGIC, 7, int)
#define CYAPCTLGETOPTJMP        _IOR(CYAPCTL_IOC_MAGIC, 8, int)
#define CYAPCTLSETCPULEDMODE    _IOW(CYAPCTL_IOC_MAGIC, 9, int)
#define CYAPCTLGETFPGAVER       _IOR(CYAPCTL_IOC_MAGIC, 10, int)
#define CYAPCTLGETGPLED         _IOR(CYAPCTL_IOC_MAGIC, 11, int)
#define CYAPCTLSETGPLED         _IOW(CYAPCTL_IOC_MAGIC, 12, int)
#define CYAPCTLGETHDLEDEN       _IOR(CYAPCTL_IOC_MAGIC, 13, int)
#define CYAPCTLSETHDLEDEN       _IOW(CYAPCTL_IOC_MAGIC, 14, int)
#define CYAPCTLSETCPULEDORNGEN  _IOW(CYAPCTL_IOC_MAGIC, 15, int)
#define CYAPCTLGETCPULEDORNGEN  _IOR(CYAPCTL_IOC_MAGIC, 16, int)
#define CYAPCTLGETVIDQUAL       _IOR(CYAPCTL_IOC_MAGIC, 17, int)
#define CYAPCTLGETSYSFPGAVER    _IOR(CYAPCTL_IOC_MAGIC, 18, int)
#define CYAPCTLGETMUXFPGAVER    _IOR(CYAPCTL_IOC_MAGIC, 19, int)
