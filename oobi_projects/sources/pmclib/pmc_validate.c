/*****************************************************************
* File: pmc_validate.c
*
* Copyright (C) 2006 Avocent Corporation
*
* www.avocent.com
*
* Description:
*     This file has the validation routines of the PM Client.
*     Library 
*
*****************************************************************/
#define PMCLIB_SRC
#define SHM_HASH
#include <pmclib.h>

#define PERM_ALL	0xffffffff
#define PERM_NONE	0

static unsigned char def_device[] = DEF_DEVICE;

// vector used to store the multiple outlets
oData *pmc_multout[MAXOutletsPS];

static int verify_userhash_out(unsigned long userhash, oData * outlet)
{
	acl *pacl = shmp->gacl;
	int ret = 0; //denny
	unsigned long devicehash=0;	
	int outN=0;

	if (outlet) {
		devicehash = Hash(outlet->ipdu->pmInfo->device);	
		outN = pmc_get_old_outlet(outlet->ipdu->id, outlet->onumber);
	}

	if (!pacl) {
		return ret;
	}

	while (pacl->userhash) {
	//	debug(PMCLIB_VALIDATE,"pacl[%x] userhash[%x] ipduhash[%x] outlethash[%x] outletnum[%d]",
	//		(int)pacl, pacl->userhash, pacl->ipduhash,pacl->outlethash,pacl->outletnum);
		if (pacl->userhash == userhash) {
			// found the username
			if (pacl->ipduhash == PERM_ALL) {
				// allow access for all PDUs
				ret = 1;
				break;
			}
			if ((pacl->ipduhash == PERM_NONE) && 
			    (pacl->outlethash == 0) && 
			    (pacl->devicehash == 0)) {
				// deny for all PDUs
				ret = 0;
				break;
			}
			if (outlet == NULL) {
				// user has permission to access outlets
				ret = 1;
				break;
			}
			if (pacl->outlethash == outlet->outnamehash) {
				// found the outlet ==> allow access
				ret = 1;
				break;
			}
			if ((pacl->ipduhash == outlet->ipdu->idhash) ||
			    (pacl->devicehash == devicehash))	{
				// found IPDU --> chech the outlet
				if (pacl->outlethash == PERM_ALL) {
					// allow access for all outlets
					ret = 1;
					break;
				}
				if ((pacl->outlethash == PERM_NONE)  &&
				    (pacl->outletnum == 0)) {
					// deny access for all outlets
					ret = 0;
					break;
				}
				if ((pacl->ipduhash && (pacl->outletnum == outlet->onumber)) ||
				     (pacl->devicehash && (pacl->outletnum == outN))) {
					// found the outlet ==> allow access
					ret = 1;
					break;
				}
			}
		}
		pacl++;
	}
	
	//debug(PMCLIB_VALIDATE,"userhash[%x] out[%x] ==> ret=%d",userhash,(int)outlet,ret);
	return ret;
}

/* pmc_validate_port
 *     There is a chain connected to the serial port and it is under 
 *     control of the PMD core.
 * return: a pointer to the chain data; NULL when the
 *         serial port is invalid or in case of error
 * Parameters:
 *   number – the number of the serial port;
 *            0 when the device name is complete (e.g. ttyS#)
 *   device – the physical port device: ttyA or ttyS
 *            Default: ttyS, when the device is NULL
 */
pmInfo *pmc_validate_port(int number, char *device)
{
	char dev[100];
	pmInfo *pminfo;

	if (shmp == NULL) return NULL;

	debug(PMCLIB_VALIDATE, "portN[%d] device[%s]", number, device);

	// when device is NULL, uses the default: ttyS
	if (device == NULL) {
		sprintf(dev,"%s%d", def_device, number);
	} else {
		if (number == 0) {
			strcpy(dev, device);
		} else {
			sprintf(dev,"%s%d", device, number);
		}
	}

	pminfo = shmp->pmInfo;

	while (pminfo != NULL) {
		if (!strcmp(dev, pminfo->device)) {
			break;
		}
		pminfo = pminfo->next;
	}

	if (pminfo) debug(PMCLIB_VALIDATE, "pminfo[%x] device[%s]", (int)pminfo, pminfo->device);

	return pminfo;
}

/* pmc_validate_alias_ipdu
 *     There is an IPDU under control of the PMD core.
 * return: a pointer to IPDU data;
 *         NULL when the IPDU is invalid.
 * Parameter:
 *   id - the alias IPDU id (i.e. !ttyS#-X)
 */
static ipduInfo *pmc_validate_alias_id(char *ipdu_id)
{
	pmInfo *pminfo = shmp->pmInfo;
	ipduInfo *ipdu = NULL;
	int ipduN;
	char *s;
	
	s = strchr(ipdu_id, '-');

	if (s == NULL) return NULL; //invalid syntaxe

	ipduN = (int)((*(s+1) - 'A') + 1);

	while (pminfo) {
		if (strlen(pminfo->device) == (s - ipdu_id - 1) && 
			!strncmp(pminfo->device, ipdu_id + 1, (s - ipdu_id - 1))) {
			break;
		}
		pminfo = pminfo->next;
	}	
	if (pminfo) {
		if (pminfo->num_ipdus < ipduN) {
			return NULL; //invalid ID
		}
		for (ipduN--,ipdu = pminfo->ipdu; (ipduN > 0) && ipdu; ipduN--,ipdu=ipdu->next);
	}
	if (!ipdu || !ipdu->ready || !ipdu->license) {
		return NULL;
	}

	return ipdu;
}

/* pmc_validate_ipdu
 *     There is an IPDU under control of the PMD core.
 * return: a pointer to IPDU data;
 *         NULL when the IPDU is invalid.
 * Parameter:
 *   id - the identification of the IPDU (i.e. the IPDU name)
 */
ipduInfo *pmc_validate_ipdu(char *ipdu_id)
{
	unsigned long hashid = Hash(ipdu_id);
	pmInfo *pminfo = shmp->pmInfo;
	ipduInfo *ipdu = NULL;

	if (*ipdu_id == '!') {
		//this is an alias id (!ttyS#-X)
		return pmc_validate_alias_id(ipdu_id);
	}

	debug(PMCLIB_VALIDATE, "ipduId[%s] hash[%x]", ipdu_id, hashid);

	while (pminfo) {
		ipdu = pminfo->ipdu;
		while (ipdu) {
			if (ipdu->ready && ipdu->license && ipdu->idhash == hashid) {
				break;
			}
			ipdu = ipdu->next;
		}
		if (ipdu) break;
		pminfo = pminfo->next;
	}

	return ipdu;
}

/* pmc_validate_ipdu_old
 *     There is an IPDU under control of the PMD core.
 * return: a pointer to IPDU data;
 *         NULL when the IPDU is invalid.
 * Parameter:
 *   port_number - number of the physical port
 *   ipdu_number - number of IPDU in the chain
 *   device – the physical port device: ttyA or ttyS. 
 *            Default: ttyS, when the device is NULL.
 */
ipduInfo *pmc_validate_ipdu_old(int port_number, int ipdu_number, char *device)
{
	pmInfo *pminfo;
	ipduInfo *ipdu = NULL;

	debug(PMCLIB_VALIDATE, "portN[%d] device[%s] ipduN[%d]", port_number, device, ipdu_number);

	if ((pminfo = pmc_validate_port(port_number, device)) == NULL) 
		return NULL;

	if (pminfo->num_ipdus < ipdu_number) 
		return NULL;

	ipdu = pminfo->ipdu;

	for(ipdu_number--; (ipdu_number > 0) && ipdu; ipdu_number--,ipdu=ipdu->next);
	
	if (!ipdu || !ipdu->ready || !ipdu->license) {
		return NULL;
	}

	debug(PMCLIB_VALIDATE, "ipdu[%x] ipduID[%s]", (int)ipdu, ipdu->id);

	return ipdu;
}

/* pmc_validate_outlet_number
 *     Validate the outlet number.
 * return: a pointer to the outlet data;
 *         NULL when the outlet number is invalid
 * Parameters:
 *   port_number - number of the physical port
 *   outlet_number - number of the outlet in the chain
 *   username - the name of the user to verify user permission
 *   device – the physical port device : ttyA or ttyS. 
 *            Default: ttyS, when the device is NULL.
 */
oData *pmc_validate_outlet_number(int port_number, int outlet_number, char *username, char *device)
{
	pmInfo *pminfo;
	ipduInfo *ipdu;
	oData *outlet = NULL;

	debug(PMCLIB_VALIDATE, "portN[%d] device[%s] outN[%d] username[%s]",
		  port_number, device, outlet_number, username);

	if ((pminfo = pmc_validate_port(port_number, device)) == NULL) 
		return NULL;
	
	ipdu = pminfo->ipdu;

	while (ipdu && ipdu->ready && ipdu->license && ipdu->outlet) {
		debug(PMCLIB_VALIDATE, "idpu[%x] outI[%d] outN[%d]", (int)ipdu, ipdu->num_outlets, outlet_number);
		if (ipdu->num_outlets >= outlet_number) {
			outlet = ipdu->outlet + (outlet_number - 1);
			break;
		}
		outlet_number -= ipdu->num_outlets;
		ipdu = ipdu->next;
	}
	if (outlet && username) {
		// verify if user has permission to access the outlet
		if (verify_userhash_out(Hash(username), outlet) == 0) {
			// deny access
			outlet = NULL;
		}
	}

	return outlet;
}

/* pmc_validate_outlet_name
 *     Validate the outlet name.
 * return: a pointer to the outlet data;
 *         NULL when the outlet name is invalid
 * Parameters:
 *   name - outlet name
 *   username - the name of the user to verify user permission
 */
oData *pmc_validate_outlet_name(char *name, char *username)
{
	int i;
	unsigned long hashout = Hash(name);
	oData *outlet = NULL;
	outletList *olist = shmp->olist;

	debug(PMCLIB_VALIDATE, "OutName[%s] username[%s]", name, username);

	while (olist) {
		for (i = 0; i < olist->status; i++) {
			if (olist->outlets[i].outnamehash == hashout) {
				outlet = &olist->outlets[i];
				break;
			}
		}
		if (outlet) break;
		olist = olist->next;
	}
	if (outlet && username) {
		// verify the user permission
		if (verify_userhash_out(Hash(username), outlet) == 0) {
			// deny access
			outlet = NULL;
		}
	}

	return outlet;
}

/* pmc_validate_outlet_ipdu
 *     Validate the outlet number in the ipdu.
 * return: a pointer to the outlet data;
 *         NULL when the outlet name is invalid
 * Parameters:
 *   ipdu_id - IPDU Id
 *   outlet_number - number of the outlet in the ipdu
 *   username - the name of the user to verify user permission
 */
oData *pmc_validate_outlet_ipdu(char *ipdu_id, int outlet_number, char *username)
{
	ipduInfo *ipdu;
	oData *outlet=NULL;

	debug(PMCLIB_VALIDATE, "ipduId[%s] outN[%d] username[%s]", ipdu_id, outlet_number, username);

	if (*ipdu_id == '!') {
		// this is alias id !ttyS#-X
		ipdu = pmc_validate_alias_id(ipdu_id);
	} else {
		ipdu = pmc_validate_ipdu(ipdu_id);
	}

	if (ipdu) {
		debug(PMCLIB_VALIDATE, "ipdu->num_out[%d] outN[%d]", ipdu->num_outlets, outlet_number);
		if ((ipdu->num_outlets >= outlet_number) && ipdu->outlet) {
			outlet = ipdu->outlet + outlet_number - 1;
			if (username) {
				// verify the user permission
				if (verify_userhash_out(Hash(username),outlet) == 0) {
					// deny access
					outlet = NULL;
				}
			}
		}
	}

	return outlet;
}

/* pmc_validate_outlet_old
 *     Validate the outlet number.
 * return: a pointer to the outlet data;
 *         NULL when the outlet name is invalid
 * Parameters:
 *   pminfo - chain data pointer
 *   outlet_number - number of the outlet in the chain
 *   username - the name of the user to verify user permission
 */
oData *pmc_validate_outlet_old(pmInfo *pminfo, int outlet_number, char *username)
{
	ipduInfo *ipdu;
	oData *outlet = NULL;

	debug(PMCLIB_VALIDATE, "outN[%d] username[%s]", outlet_number, username);

	ipdu = pminfo->ipdu;

	while (ipdu && ipdu->ready && ipdu->license && ipdu->outlet) {
		if (ipdu->num_outlets > outlet_number - 1) {
			outlet = ipdu->outlet + (outlet_number - 1);
			break;
		}
		outlet_number -= ipdu->num_outlets;
		ipdu = ipdu->next;
	}
	if (outlet && username) {
		// verify if user has permission to access the outlet
		if (verify_userhash_out(Hash(username), outlet) == 0) {
			// deny access
			outlet = NULL;
		}
	}

	return outlet;
}

/* pmc_validate_outlet_sysname
 *     Validate the outlet system name.
 * return: a pointer to the outlet data;
 *         NULL when the outlet name is invalid
 * Parameters:
 *   ipdu - IPDU data pointer
 *   name - outlet system name
 *   username - the name of the user to verify user permission
 */
oData *pmc_validate_outlet_sysname(ipduInfo *ipdu, char *sysname, char *username)
{
	int i;
	oData *outlet = NULL;

	debug(PMCLIB_VALIDATE, "OutSysname[%s] username[%s]", sysname, username);

	if (!ipdu || !ipdu->ready || !ipdu->license || !ipdu->outlet) {
		return NULL;
	}

	for (i = 0; i < ipdu->num_outlets; i ++) {
		if (ipdu->outlet[i].sflag & OUTLET_CAP_SYSNAME && 
			!strcmp(ipdu->outlet[i].sysname, sysname)) {
			outlet = &ipdu->outlet[i];
			break;
		}
	}
	if (outlet && username) {
		// verify the user permission
		if (verify_userhash_out(Hash(username),outlet) == 0) {
			// deny access
			outlet = NULL;
		}
	}

	return outlet;
}

/* pmc_validate_multout_name
 *     Verify that name is the name of an outlet group.
 * return: an array of pointers to the outlets that 
 *         belong to the group (last pointer is NULL);
 *         NULL otherwise
 * Parameters:
 *   group_name - the group name used to identify the group of outlets
 *   username - the name of the user to verify user permission
 */
oData **pmc_validate_multout_name(char *group_name, char *username)
{
	int i, index=0;
	outletgroup *ngrp = shmp->oGroups;
	oData *po;

	debug(PMCLIB_VALIDATE, "group_name[%s] username[%s]", group_name, username);

	while (ngrp) {
		if (!strcmp(ngrp->groupname, group_name)) {
			debug(PMCLIB_VALIDATE, "group_name[%s] numelem[%d]", group_name, ngrp->numOfMembers);
			for (i = 0; i < ngrp->numOfMembers; i++) {
				po = NULL;
				if (!ngrp->groupmembers[i].outletnum) {
					po = pmc_validate_outlet_name(ngrp->groupmembers[i].outlname, username);
				} else {
					if ((ngrp->groupmembers[i].ipduname[0] == '!') && 
						(strchr(ngrp->groupmembers[i].ipduname, '-') == NULL)) { //by serial port
						po = pmc_validate_outlet_number (0, ngrp->groupmembers[i].outletnum,
														 username, &ngrp->groupmembers[i].ipduname[1]);
					} else {
						po = pmc_validate_outlet_ipdu(ngrp->groupmembers[i].ipduname, 
													  ngrp->groupmembers[i].outletnum, username);
					}
				}
				if (po)  {
					pmc_multout[index++] = po;
				} else {
					index = 0;
					break;
				}
				debug(PMCLIB_VALIDATE,"po[%x] i[%d] index[%x]",(int)po,i,index);
			}
			break;
		}
		ngrp = ngrp->next;
	}

	if (index) {
		pmc_multout[index] = NULL; 
		return pmc_multout;
	} else
		return NULL;
}

/* pmc_validate_multout_out
 *     Verify that outlet belongs to an outlet group.
 * return: the name of the group;
 *         NULL otherwise
 * Parameters:
 *   outlet - outlet data pointer
 */
char *pmc_validate_multout_out(oData *outlet)
{
	int i;
	outletgroup *ngrp = shmp->oGroups;
	oData *po;

	while (ngrp) {
		for (i = 0; i < ngrp->numOfMembers; i++) {
			if (!ngrp->groupmembers[i].outletnum) {
				if (!strcmp(ngrp->groupmembers[i].outlname, outlet->name)) {
					return ngrp->groupname;
				}
			} else {
				if (ngrp->groupmembers[i].ipduname[0] != '!') {
					if ((ngrp->groupmembers[i].outletnum == outlet->onumber) &&
				    	(!strcmp(ngrp->groupmembers[i].ipduname,outlet->ipdu->id))) {
						return ngrp->groupname;
					}
				} else if (strchr(ngrp->groupmembers[i].ipduname, '-')) {
					po = pmc_validate_outlet_ipdu(ngrp->groupmembers[i].ipduname, 
												  ngrp->groupmembers[i].outletnum, NULL);
					if (po == outlet) {
						return ngrp->groupname;
					}
				} else { //by serial port
					po = pmc_validate_outlet_number (0, ngrp->groupmembers[i].outletnum,
													 NULL, &ngrp->groupmembers[i].ipduname[1]);
					if (po == outlet) {
						return ngrp->groupname;
					}
				}
			}
		}
		ngrp = ngrp->next;
	}

	return NULL;
}

/* pmc_validate_access_one
 *     Verify the permission of the regular user to manage outlets.
 * return: 1 if the user has permission to access outlets,
 *         0 otherwise
 * Parameters:
 *  username - the name of the user to verify user permission
 */
int pmc_validate_access_one(char *username)
{
	return verify_userhash_out(Hash(username), NULL);
}

/* pmc_validate_access_ipdu
 *     Verify the permission of a regular user to manage outlets from
 *     the IPDU pointed by IPDU data pointer.
 * return: an array of pointers to the outlets that the user
 *         can manage (last pointer is NULL);
 *         NULL otherwise
 * Parameters:
 *   username - the name of the user to verify user permission
 *   ipdu - IPDU data pointer
 */
oData **pmc_validate_access_ipdu(char *username, ipduInfo *ipdu)
{
	unsigned long userhash;
	unsigned long devicehash;
	int outN;
	int index = 0, i;
	acl *pacl;

	debug(PMCLIB_VALIDATE, "ipdu[%x] ipduID[%s] username[%s]", (int)ipdu, ipdu->id,username);

	if (!ipdu || !ipdu->ready|| !ipdu->license ) {
		return NULL;
	}

	if (username == NULL) {
		for (index=0; index < ipdu->num_outlets; index++) {
			pmc_multout[index] = ipdu->outlet + index;
		}

	} else {

		if (!shmp->gacl) {
			return NULL;
		}
		userhash = Hash(username);
		devicehash = Hash(ipdu->outlet->ipdu->pmInfo->device);
		outN = pmc_get_old_outlet(ipdu->id, 1);

		for (i = 0; i < ipdu->num_outlets; i++) {
			pacl = shmp->gacl;
			while (pacl->userhash) {
				if (pacl->userhash == userhash) {
					int ok = 0;
					if (pacl->ipduhash == PERM_NONE &&
						pacl->devicehash == 0 && 
						pacl->outlethash == 0) {
						// deny
						break;
					} else if (pacl->ipduhash == PERM_ALL) {
						ok = 1;
					} else if (pacl->outlethash == (ipdu->outlet + i)->outnamehash) {
						ok = 1;
					} else if (pacl->ipduhash != ipdu->idhash &&
							   pacl->devicehash != devicehash) {
						// bypass
					} else if (pacl->outlethash == PERM_ALL) {
						ok = 1;
					} else if (pacl->outlethash == PERM_NONE &&
							   pacl->outletnum == 0) {
						// deny
						break;
					} else if (pacl->ipduhash && pacl->outletnum == i + 1) {
						ok = 1;
					} else if (pacl->devicehash && pacl->outletnum == i + outN) {
						ok = 1;
					}
					if (ok) {
						pmc_multout[index++] = (ipdu->outlet + i);
						break;
					}
				}
				pacl++;
			}
		}
	}

	if (index) {
		pmc_multout[index] = NULL;
		return pmc_multout;
	} else {
		return NULL;
	}
}

// convert the IPDU Id to IPDU index in the chain
int pmc_get_old_ipdu(char *id)
{
	unsigned long idhash = Hash(id);
	pmInfo *chain = shmp->pmInfo;
 	ipduInfo *ipdu;
	int ipdun;

	while (chain) {
		ipdun = 1;
		ipdu = chain->ipdu;
		while (ipdu) {
			if (ipdu->ready && ipdu->license && ipdu->idhash == idhash) {
				return ipdun;
			}
			ipdun ++;
			ipdu = ipdu->next;
		}
		chain = chain->next;
	}
	return 0;
}

// convert <IPDU ID>[outN] to outlet number in the chain
int pmc_get_old_outlet(char *id, int outlet)
{
	unsigned long idhash = Hash(id);
	pmInfo *chain = shmp->pmInfo;
 	ipduInfo *ipdu;
	int outn;

	while (chain) {
		outn = 0;
		ipdu = chain->ipdu;
		while (ipdu && ipdu->ready && ipdu->license && ipdu->outlet) {
			if (ipdu->idhash == idhash) {
				return (outn + outlet);
			}
			outn += ipdu->num_outlets;
			ipdu = ipdu->next;
		}
		chain = chain->next;
	}
	return 0;
}

// verify the user permission
// ret = 0 : denny,  1 : allow
int pmc_check_userperm(oData *outlet, char *username)
{
	return verify_userhash_out(Hash(username), outlet);
}


// PMCLIB Extension below this point


/* pmc_validate_phase
 *     Find a certain phase under a given IPDU.
 * return: a pointer to phase data;
 *         NULL when the phase is not found.
 * Parameter:
 *         ipdu - IPDU data pointer
 *   phase_name - phase name (actually, system name: X, Y or Z)
 */
phaseInfo *pmc_validate_phase(ipduInfo *ipdu, char *phase_name)
{
	int i;
	phaseInfo *phptr = NULL;

	if (!ipdu || !ipdu->ready || !ipdu->license) return NULL;

	debug(PMCLIB_VALIDATE, "ipduId[%s] phase[%s]", ipdu->id, phase_name);

	for (i=0; i < ipdu->num_phases; i++) {
		if (strcmp(ipdu->phase[i].sysname, phase_name) == 0) {
			phptr = ipdu->phase + i;
			break;
		}
	}

	return phptr;
}

/* pmc_validate_circuit
 *     Find a certain circuit under a given IPDU.
 * return: a pointer to circuit data;
 *         NULL when the circuit is not found.
 * Parameter:
 *           ipdu - IPDU data pointer
 *   circuit_name - circuit name (actually, system name: A, B, XY1, YZ, ...)
 */
circuitInfo *pmc_validate_circuit(ipduInfo *ipdu, char *circuit_name)
{
	int i;
	circuitInfo *cirptr = NULL;

	if (!ipdu || !ipdu->ready || !ipdu->license) return NULL;

	debug(PMCLIB_VALIDATE, "ipduId[%s] circuit[%s]", ipdu->id, circuit_name);

	for (i=0; i < ipdu->num_circuits; i++) {
		if (strcmp(ipdu->circuit[i].sysname, circuit_name) == 0) {
			cirptr = ipdu->circuit + i;
			break;
		}
	}

	return cirptr;
}

/* pmc_validate_envmon
 *     Find a certain environmental monitor under a given IPDU.
 * return: a pointer to environmental monitor data;
 *         NULL when the environmental monitor is not found.
 * Parameter:
 *           ipdu - IPDU data pointer
 *    envmon_name - environmental monitor name (actually, system name: T1, H2, ...)
 */
envMonInfo *pmc_validate_envmon(ipduInfo *ipdu, char *envmon_name)
{
	int i;
	envMonInfo *envptr = NULL;

	if (!ipdu || !ipdu->ready || !ipdu->license) return NULL;

	debug(PMCLIB_VALIDATE, "ipduId[%s] envMon[%s]", ipdu->id, envmon_name);

	for (i=0; i < ipdu->num_envMons; i++) {
		if (strcmp(ipdu->envMon[i].sysname, envmon_name) == 0) {
			envptr = ipdu->envMon + i;
			break;
		}
	}

	return envptr;
}
