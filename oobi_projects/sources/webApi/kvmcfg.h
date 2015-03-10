/***************************************************************** 
 * File: kvmconfig.h 
 * 
 * Copyright (C) 2003 Cyclades Corporation 
 * 
 * www.cyclades.com 
 * 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either version 
 * 2 of the License, or (at your option) any later version. 
 * 
 * Description: This part contains the definitions and prototypes
 *	for the kvmconfig.c module.
 * 
 ****************************************************************/ 

#define	KVM_C_LIST		'L'
#define KVM_C_INT		'I'
#define	KVM_C_STR		'S'
#define	KVM_C_PORT		'P'
#define	KVM_C_NUMLIST		'O'
#define	KVM_C_GROUP		'G'
#define	KVM_C_SEQ1		'Q'
#define	KVM_C_SEQ2		'E'
#define	KVM_C_SEQ3		'M'
#define	KVM_C_NEWSEQ		'N'
#define KVM_C_LOCKOUT           'T'
/*===============================================================
 *		Internal structure definition 
 *==============================================================*/
struct kvm_lst_def {
	char *name;			/* name in the config file */
	char *label;			/* label in webs */
	int value;			/* value in the config structure */
};

struct kvm_conf_def {
	char *name;			/* name in the config file */
	char *label;			/* label in the webs/cli */
	char *slabel;			/* short label for OSD */
	int type;			/* type of the parameter */
	struct kvm_lst_def *opts;		/* list of options */
	unsigned int min;		/* minimum value */
	unsigned int max;		/* maximum value */
	char *defvalue;			/* default value */
	int offs;			/* offset in the structure */
	int (*checkshow)(void *ptr);	/* routine to check if the parameter 
					should be shown in the web/OSD 
					interface*/
	int (*checkfile)(char *);	/* routine to check parameter in file */
	int (*checkstruct)(void *ptr, void *pval);	/* routine to check in the structure */
};

/*===============================================================
 * Prototypes 
 *==============================================================*/
char *getconfig(char *ptr, struct kvm_conf_def *x, char *buf);
int kvm_setconfig (char *ptr, struct kvm_conf_def *x, char *a);
int SetOutletList(void *ptr, char *inpline, int device, int min, int max);
void defaultconf (struct kvm_conf_def *cfg, char *ptr);
int compareconf (struct kvm_conf_def *cfg, char *ptr1, char *ptr2);
void copyconf(struct kvm_conf_def *cfg, char *dst, char *src);
void releaseconf(struct kvm_conf_def *cfg, char *ptr);

char *getsequence (char *ptr, char *buf);
char *getnumlist(char *ptr, char *buf);
int setstr(char *ptr, char *val);
int check_numoutlets();
int pm_get_status();
int is_valid_ip(const char *value);
