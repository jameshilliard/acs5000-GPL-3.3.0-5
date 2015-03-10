/*-----------------------------------------------------------------------------
 *	Copyright (C) Cyclades Corporation, 1997.
 *	All rights reserved
 *
 *	TS1000 - RAS
 *
 *	This file:	cpld.h
 *	Description:	This file contains the definitions and prototypes 
 *			related to the CPLD programming
 *			
 *	
 *	Complies with Cyclades SW Coding Standard rev 1.2.
 *-----------------------------------------------------------------------------
 */
 
/*-----------------------------------------------------------------------------
 *	Change History
 *-----------------------------------------------------------------------------
 * Oct/25/2000	V 1.0.0		Edson Seabra
 *	Initial Implementation
 *
 *-----------------------------------------------------------------------------
 */

#define DB_UART_ADDR_SIZE		0x1000
#define DB_CPLD_ADDR_SIZE		0x1000
#define DB_FPGA_ADDR_SIZE		0x1000
#define DB_FPGA_PC_ADDR_SIZE	0x1000
#define DB_SYSFPGA_ADDR_SIZE	0x1000
#define DB_MUXFPGA_ADDR_SIZE	0x1000
 
/* Description of bits in fpga_jp field in the cpld.
*/
#define FPGA_JP_PCMCIA          0x02 // bit 6 (Big Endian)


/* Description of bits in fpga_pc_ctl field in the cpld.
   Bits 0-3 for slot A, bits 4-7 for slot B.
*/
#define PCMCIA_VCC_33	        0x20
#define PCMCIA_VCC_50	        0x10
#define PCMCIA_VPP_12	        0x80
#define PCMCIA_VPP_VCC	        0x40
#define PCMCIA_VCC_MASK	        0x30
#define PCMCIA_VPP_MASK	        0xC0

/* Description of bits in fpga_pc_sts field in the cpld.
*/
#define FPGA_PCMCIA_ENABLE	    0x80 // bit 0 

/* CPLD registers */ 
typedef struct _cpld_regs {
	unsigned char reset_enable1;		// 0x00
	unsigned char clock_enable1;		// 0x01
	unsigned char clock_source1;		// 0x02
	unsigned char cpld_misc;			// 0x03
	unsigned char intr_enable1;			// 0x04
	unsigned char intr_enable2;			// 0x05
	unsigned char intr_enable3;			// 0x06
	unsigned char intr_enable4;			// 0x07
	unsigned char intr_status1;			// 0x08
	unsigned char intr_status2;			// 0x09
	unsigned char intr_status3;			// 0x0A
	unsigned char intr_status4;			// 0x0B
	unsigned char fpga_id;				// 0x0C
	unsigned char fpga_jp;				// 0x0D
	unsigned char fpga_cfst;			// 0x0E
	unsigned char fpga_led_ctl;			// 0x0F
	unsigned char reset_enable2;		// 0x10
	unsigned char clock_enable2;		// 0x11
	unsigned char clock_source2;		// 0x12
	unsigned char reserve2;				// 0x13
	unsigned char intr_enable5;			// 0x14
	unsigned char intr_enable6;			// 0x15
	unsigned char reserve3[2];			// 0x16
	unsigned char intr_status5;			// 0x18
	unsigned char intr_status6;			// 0x19
	unsigned char reserve4[6];			// 0x1A
	unsigned char bsmux;				// 0x20
	unsigned char fpga_pc_ctl;			// 0x21
	unsigned char fpga_pc_sts;			// 0x22
	unsigned char reserve5[4];			// 0x23
	unsigned char id_cpld;				// 0x27
} cpld_regs;

/* Serial Expander UART registers */
typedef union 
{
	struct uart_read  {
		unsigned char	rhr;
		unsigned char	space;
		unsigned char	isr;		// SSE8-V174
		unsigned char	lcr;
		unsigned char	mcr;
		unsigned char	lsr;
		unsigned char	msr;
		unsigned char	spr;
	} r;
											
	struct uart_write  {
		unsigned char	thr;
		unsigned char	ier;
		unsigned char	fcr;
		unsigned char	lcr;
		unsigned char	mcr;
		unsigned char	space[2];
		unsigned char	spr;
    } w;
	
	struct uart_special {
		unsigned char	dll;
		unsigned char	dlm;
		unsigned char	efr;
		unsigned char	space;
		unsigned char	xon1;
		unsigned char	xon2;
		unsigned char	xoff1;
		unsigned char	xoff2;
	} s;
} UART_REGS;

typedef struct _fpga_regs {
	unsigned char fpga_async;
	unsigned char fpga_wan_reg1;
	unsigned char fpga_wan_reg2;
	unsigned char reserved1;
	unsigned char fpga_cf;
	unsigned char reserved2;
	unsigned char fpga_int;
	unsigned char fpga_dimm;
	unsigned char reserved3;
	unsigned char fpga_led;
	unsigned char fpga_jpr;
	unsigned char fpga_id;
} fpga_regs;

/* PCMCIA registers */
typedef struct _fpga_pc_regs {
	unsigned char fpga_pc_misc;	// Controls PCMCIA IO's window size
	unsigned char reserved1[7];
	unsigned char fpga_pc_stat;	// Status - read only
	unsigned char reserved2[6];
	unsigned char fpga_pc_id;	// PCMCIA FPGA ID - read only 
} fpga_pc_regs;

/* FPGA MUX register */
typedef struct _muxfpga_regs {
	unsigned char misc;
	unsigned char uart_int;
	unsigned char vid_ctl_1;
	unsigned char vid_sts_1;
	unsigned char vid_gain_1;
	unsigned char vid_eq_1;
	unsigned char vid_ctl_2;
	unsigned char vid_sts_2;
	unsigned char vid_gain_2;
	unsigned char vid_eq_2;
	unsigned char kvm_sel_1;
	unsigned char kvm_sel_2;
	unsigned char adm_sel;
	unsigned char osd_1;
	unsigned char osd_2;
	unsigned char id;
	unsigned char hlo_lsb_1;
	unsigned char hlo_msb_1;
	unsigned char hhi_lsb_1;
	unsigned char hhi_msb_1;
	unsigned char vlo_lsb_1;
	unsigned char vlo_msb_1;
	unsigned char vhi_lsb_1;
	unsigned char vhi_msb_1;
	unsigned char hlo_lsb_2;
	unsigned char hlo_msb_2;
	unsigned char hhi_lsb_2;
	unsigned char hhi_msb_2;
	unsigned char vlo_lsb_2;
	unsigned char vlo_msb_2;
	unsigned char vhi_lsb_2;
	unsigned char vhi_msb_2;
	unsigned char misc_2;
	unsigned char uart_int_2;
} muxfpga_regs;

extern volatile cpld_regs *cpld;

extern volatile void	*uarts;

extern volatile fpga_regs *fpga;

extern volatile muxfpga_regs *muxfpga;

extern cpld_regs * get_cpld(void);
