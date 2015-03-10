/*
 * auth.h	Header file for the authentication stuff.
 *		We only use radius, but anyway this is
 *		a try to hide the authentication protocol
 *		from the rest of the system.
 *
 * Version:	@(#)auth.h  1.00  05-Jul-1996  miquels@cistron.nl
 *
 */

#ifndef __auth_types_h
#define __auth_types_h

/*
 *	Types of authentication
 *
*/

#define AUTH_NONE			0x0000
#define AUTH_REMOTE			0x0010

#define AUTH_RADIUS			0x1000
#define AUTH_RADIUS_LOCAL		0x1001
#define AUTH_RADIUS_DOWN_LOCAL		0x1002

#define AUTH_TacacsPlus			0x2000
#define AUTH_TacacsPlus_LOCAL		0x2001
#define AUTH_TacacsPlus_DOWN_LOCAL	0x2002

#ifdef KRBenable
#define AUTH_KERBEROS			0x3000
#define AUTH_KERBEROS_LOCAL	0x3001
#define AUTH_KERBEROS_DOWN_LOCAL	0x3002
#endif

#define AUTH_LDAP			0x4000
#define AUTH_LDAP_LOCAL			0x4001
#define AUTH_LDAP_DOWN_LOCAL		0x4002

#ifdef NISenable
#define AUTH_NIS					0x5000
#define AUTH_NIS_LOCAL				0x5001
#define AUTH_NIS_DOWN_LOCAL			0x5002
#endif

#ifdef SMBenable
#define AUTH_SMB					0x6000
#define AUTH_SMB_DOWN_LOCAL			0x6001
#endif

#ifdef OTPenable
#define AUTH_OTP			0X0007
#endif

#define AUTH_LOCAL			0x0080
#define AUTH_LOCAL_RADIUS		0x8001
#define AUTH_LOCAL_TacacsPlus		0x8002
#ifdef NISenable
#define AUTH_LOCAL_NIS				0x8005
#endif


#ifdef CYCLADES
#define AUTH_BIOMETRIC				0x70
#endif

#define AUTH_SERVER			0xF000

#endif	/* __auth_types_h */
