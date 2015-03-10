/**
 *  kvm_auxport.h  -  Prototypes and definitions for kvm_auxport.c
 *  Copyright (C), 2004 Cyclades Corporation
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Written by:
 *          Rodrigo Parra Novo <rodrigo.novo@cyclades.com>
 *
 *  This file is part of webApi
 */

/**
 * $Id: kvm_auxport.h,v 1.9 2007/09/27 19:26:52 acyr Exp $
 */

#ifndef KVM_AUXPORT_H
#define KVM_AUXPORT_H

/**
 * return codes
 */
typedef enum {
	KAP_OK = 0,
	KAP_NULL = -1,
	KAP_EFILE = -2,
	KAP_EMEM = -3,
	KAP_EPARSE = -4
} kvm_auxport_result;

/**
 * protocol types
 */
typedef enum {
	KVM_AUXPORT_PROTOCOL_IPDU,
	KVM_AUXPORT_PROTOCOL_PPP_ONLY,
	KVM_AUXPORT_PROTOCOL_LOGIN	
} kvm_auxport_protocol;

/**
 * parity types
 */
typedef enum {
	KVM_AUXPORT_PARITY_NONE,
	KVM_AUXPORT_PARITY_ODD,
	KVM_AUXPORT_PARITY_EVEN
} kvm_auxport_parity;

/**
 * flow types
 */
typedef enum {
	KVM_AUXPORT_FLOW_NONE,
	KVM_AUXPORT_FLOW_SOFT,
	KVM_AUXPORT_FLOW_HARD
} kvm_auxport_flow;

/**
 * PM types
 */
typedef enum {
#ifdef OEM3
	KVM_AUXPORT_PM_APC,
#else
	KVM_AUXPORT_PM_CYCLADES,
	KVM_AUXPORT_PM_SERVERTECH,
	KVM_AUXPORT_PM_SPC,
#endif
	KVM_AUXPORT_PM_AUTO
} kvm_auxport_pm;

/**
 * All AUX Port is kept here
 */
typedef struct kvm_auxport_t {
	int auxidx;
	int enable;
	char *filename;
#ifdef PMDNG
	char *extra_config_pm;
	long extra_config_pm_size;
	char *extra_config_ps;
	long extra_config_ps_size;
#else
	char *extra_config;
	long extra_config_size;
#endif
	kvm_auxport_protocol protocol;
	kvm_auxport_parity parity;
	kvm_auxport_flow flow;
	int speed;
	int datasize;
	int stopbits;
	char *initchat;
	char *autoppp;
	char *tty;
	/**
	 * the two variables below have fixed values,
	 * but we'll keep them, "just in case"
	 */
	kvm_auxport_pm pmtype;
	int pmNumOfOutlets;
} *kvm_auxport;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * kvm_auxport.c
 */
kvm_auxport kvm_auxport_new (void);
#ifdef PMDNG
kvm_auxport_result kvm_auxport_pmdps_config_write (kvm_auxport kap, char *pm_conf_file, char *ps_conf_file);
kvm_auxport_result kvm_auxport_pmdps_config_read (kvm_auxport kap, char *pm_conf_file, char *ps_conf_file);
#else
kvm_auxport_result kvm_auxport_config_read (kvm_auxport kap, char *config_file);
kvm_auxport_result kvm_auxport_config_write (kvm_auxport kap, char *config_file);
#endif
kvm_auxport_result kvm_auxport_free (kvm_auxport kap);
const char *kvm_auxport_strerror (kvm_auxport_result error_number);
char* kvm_auxport_format_mstring (char* dst, char* src, int len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* KVM_AUXPORT_H */
