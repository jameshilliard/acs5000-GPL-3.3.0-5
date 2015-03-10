/**
 *  kvm_cascade.h  -  Prototypes and definitions for kvm_cascade.c
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
 * $Id: kvm_cascade.h,v 1.2 2006/02/24 19:21:01 christineq Exp $
 */

#ifndef KVM_CASCADE_H
#define KVM_CASCADE_H

/**
 * definitions
 */
#define KC_OK 0
#define KC_NULL -1
#define KC_EFILE -2
#define KC_EMEM -3
#define KC_EPARSE -4
#define KC_INTERNAL -5

enum {
	KVM_CASCADE_PHYSICAL,
	KVM_CASCADE_NETWORK
};

struct kvm_port_t {
	char *active;
	char *alias;
	char *lockoutmacro;
	char *pmoutletstr;
	char *pmdevice1;
	char *pmdevice2;
	int pmoutlet1;
	int pmoutlet2;
	int tcpport;
	int lclagc;
	int lclbright;
	int lclcontr;
	int remagc;
	int rembright;
	int remcontr;
	int ipbright[4];
	int ipcontr[4];
};

struct kvm_cascade_dev_t {
	char *name;
	char *parent;
	char *hostname;
	int numports;
	int type;
	int user1;
	int user2;
	struct kvm_cascade_dev_t *next;
	struct kvm_port_t *ports;
};

struct kvm_cascade_t {
	char *filename;
	int numdevices;
	struct kvm_cascade_dev_t *head;
	char *extra_config;
	long extra_config_size;
};
typedef struct kvm_cascade_t *kvm_cascade;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * kvm_cascade.c
 */
kvm_cascade kvm_cascade_new (void);
int kvm_cascade_config_read (kvm_cascade kc, char *config_file);
int kvm_cascade_config_write (kvm_cascade kc, char *config_file);
int kvm_ports_config_read (kvm_cascade kc, char *config_file);
int kvm_ports_config_write (kvm_cascade kc, char *config_file);
int kvm_cascade_free (kvm_cascade kc);
const char *kvm_cascade_strerror (int error_number);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* KVM_CASCADE_H */
