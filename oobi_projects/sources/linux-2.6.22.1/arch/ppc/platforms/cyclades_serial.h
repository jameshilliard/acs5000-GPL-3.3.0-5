/*-----------------------------------------------------------------------------
 *	Copyright (C) Cyclades Corporation, 1997.
 *	All rights reserved
 *
 *	TSxK - RAS
 *
 *	Complies with Cyclades SW Coding Standard rev 1.2.
 *-----------------------------------------------------------------------------
 * included in include/asm/serial.h
 */

#ifndef ASM_PPC_CYCLADES_SERIAL_H
#define ASM_PPC_CYCLADES_SERIAL_H

#define SERIAL_IRQ0_VALID

#ifdef CONFIG_SERIAL_DETECT_IRQ
#define STD_COM_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST | ASYNC_AUTO_IRQ)
#define STD_COM4_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_AUTO_IRQ)
#else
#define STD_COM_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST)
#define STD_COM4_FLAGS ASYNC_BOOT_AUTOCONF
#endif

#ifdef CONFIG_SERIAL_MANY_PORTS
#define FOURPORT_FLAGS ASYNC_FOURPORT
#define ACCENT_FLAGS 0
#define BOCA_FLAGS 0
#define HUB6_FLAGS 0
#endif

#define UART_XON1	4
#define UART_XON2	5
#define UART_XOFF1	6
#define UART_XOFF2	7

#define CYCLADES_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_SHARE_IRQ)
#define BASE_BAUD ( 14745600 / 16 )

#define CYCLADES_SERIAL_PORT_DFNS		\
 {0, 0, 0, 0xff, 0, 0, 0, 0, 0, 0, 0, 1},  /* ttyS00 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS01 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS02 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS03 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS04 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS05 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS06 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS07 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS08 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS09 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS10 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS11 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS12 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS13 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS14 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS15 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS16 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS17 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS18 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS19 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS20 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS21 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS22 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS23 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS24 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS25 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS26 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS27 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS28 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS29 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS30 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS31 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS32 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS33 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS34 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS35 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS36 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS37 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS38 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS39 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS40 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS41 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS42 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS43 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS44 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS45 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS46 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS47 */	\
 {0, BASE_BAUD, 0, 0xff, 0, 0x10000 },  /* ttyS48 */	\
 {0, 0, 0, 0xff, 0, 0 }, 		/* ttyM1  */	\
 {0, 0, 0, 0xff, 0, 0 }			/* ttyM2  */

#define SERIAL_PORT_DFNS	CYCLADES_SERIAL_PORT_DFNS

#endif 
