
#define TRUE	1
#define FALSE	0
#define SUCCESS 0
#define FAIL	-1

#define	ULONG		unsigned long
#define USHORT		unsigned short
#define BOOLEAN		unsigned char

// Definition for AX8819x
#define	IOBase		(e8390_base)
#define a19x_CR 	(IOBase)
// Read Page 0
#define a19x_CLDA0	(IOBase+0x01)
#define a19x_CLDA1	(IOBase+0x02)
#define a19x_BNRY	(IOBase+0x03)
#define a19x_TSR	(IOBase+0x04)
#define a19x_NCR	(IOBase+0x05)
#define a19x_CURR	(IOBase+0x06)
#define a19x_ISR	(IOBase+0x07)
#define a19x_CRDA0	(IOBase+0x08)
#define a19x_CRDA1	(IOBase+0x09)
#define a19x_RSV01	(IOBase+0x0A)
#define a19x_RSV02	(IOBase+0x0B)
#define a19x_RSR	(IOBase+0x0C)
#define a19x_CNTR0	(IOBase+0x0D)
#define a19x_CNTR1	(IOBase+0x0E)
#define a19x_CNTR2	(IOBase+0x0F)
// Write Page 0
#define a19x_PSTART	a19x_CLDA0
#define a19x_PSTOP	a19x_CLDA1
#define a19x_TPSR	a19x_TSR
#define a19x_TBCR0	a19x_NCR
#define a19x_TBCR1	a19x_CURR
#define a19x_RSAR0	a19x_CRDA0
#define a19x_RSAR1	a19x_CRDA1
#define a19x_RBAR0	a19x_RSV01
#define a19x_RBAR1	a19x_RSV02
#define a19x_RCR	a19x_RSR
#define a19x_TCR	a19x_CNTR0
#define a19x_DCR	a19x_CNTR1
#define a19x_IMR	a19x_CNTR2
// Read/Write Page 1
#define a19x_PAR0	a19x_CLDA0
#define a19x_PAR1	a19x_CLDA1
#define a19x_PAR2	a19x_BNRY
#define a19x_PAR3	a19x_TSR
#define a19x_PAR4	a19x_NCR
#define a19x_PAR5	a19x_CURR
#define a19x_MAR0	a19x_CRDA0
#define a19x_MAR1	a19x_CRDA1
#define a19x_MAR2	a19x_RSV01
#define a19x_MAR3	a19x_RSV02
#define a19x_MAR4	a19x_RSR
#define a19x_MAR5	a19x_CNTR0
#define a19x_MAR6	a19x_CNTR1
#define a19x_MAR7	a19x_CNTR2
// independent port
#define a19x_DATA	(IOBase+0x10)
#define a19x_IFGS1	(IOBase+0x12)
#define a19x_IFGS2	(IOBase+0x13)
#define a19x_MEMR	(IOBase+0x14)	//peripheral(SROM, PHY) data access
#define a19x_TEST	(IOBase+0x15)
#define a19x_IFG	(IOBase+0x16)
#define a19x_RESET	(IOBase+0x1F)

#define MII_MDO_BIT_POSITION	3
#define MII_MDO_MASK		8
#define MII_WRITE		0
#define MII_READ		2
#define MII_CLK 		1

//***************************************************************************
// Write data to MII
//***************************************************************************
void
A19xWriteMii(
        struct net_device *dev,
	ULONG ulData,
	USHORT uDataSize
	)
{
        long e8390_base = dev->base_addr;
	for (; uDataSize > 0; uDataSize--)
	{
		ULONG	ulDataBit;

		ulDataBit = (ulData >> (31 - MII_MDO_BIT_POSITION)) & MII_MDO_MASK;
		outb_p(MII_WRITE |ulDataBit, a19x_MEMR);
		outb_p(MII_WRITE |MII_CLK |ulDataBit, a19x_MEMR);
		ulData <<= 1;
	}
}

#define MII_WRITE_TS			2	//

//***************************************************************************
// Set MII to three-state
//***************************************************************************
#define MiiOut3State	{outb_p(MII_WRITE_TS, a19x_MEMR); \
			outb_p(MII_WRITE_TS |MII_CLK, a19x_MEMR);}

#define PHY_ADDR_ALIGN		23	// shift
#define REG_ADDR_ALIGN		18	// shift
#define PRE			((ULONG)(0xFFFFFFFF))
#define MII_READ_FRAME		((ULONG)(0x60000000))
#define MII_WRITE_FRAME 	((ULONG)(0x50020000))
#define MII_MDI_BIT_POSITION	2
#define MII_MDI_MASK		4
#define MII_READ_DATA_MASK	MII_MDI_MASK

#define MiiPhyCtrlReservedBitsMask	((USHORT) 0x007F)
#define MiiPhyStatReservedBitsMask	((USHORT) 0x07C0)
#define MiiPhyNwayReservedBitsMask	((USHORT) 0x1C00)
#define MiiPhyNwayExpReservedBitsMask	((USHORT) 0xFFE0)

//Data Structure holding PHY's registers mask
static const USHORT PhyRegsReservedBitsMasks[] = {

	MiiPhyCtrlReservedBitsMask,	// Control reg reserved bits mask
	MiiPhyStatReservedBitsMask,	// Status reg reserved bits
	0,				// PhyID reserved bits mask
	0,				// PhyID reserved bits mask
	MiiPhyNwayReservedBitsMask,	// Nway Local ability reserved bits mask
	MiiPhyNwayReservedBitsMask,	// Nway Partner ability reserved bits mask
	MiiPhyNwayExpReservedBitsMask,	// Nway Expansion
	0,0,0,0,0,0,0,0,0,0,0,0,0,	// Other regs
	0,0,0,0,0,0,0,0,0,0,0,0		// Other regs

	};

//***************************************************************************
// Read the value of the MII register
//***************************************************************************
BOOLEAN
MiiReadRegister(
                struct net_device *dev,
		USHORT	uPhyAddr,
		USHORT	uRegNum,
		USHORT	*puRegData
		)
{
        long e8390_base = dev->base_addr;

	ULONG	ulCommand = ((ULONG)uPhyAddr <<PHY_ADDR_ALIGN)
			|((ULONG)uRegNum <<REG_ADDR_ALIGN)
			|MII_READ_FRAME;
	USHORT	uBitsOfUshort = sizeof(USHORT) * 8;
	USHORT	i;

	*puRegData = 0;

	A19xWriteMii(dev,0, 16);
	A19xWriteMii(dev,PRE, (USHORT)(2*uBitsOfUshort));
	A19xWriteMii(dev,ulCommand, (USHORT)(uBitsOfUshort-2));
	MiiOut3State

	if (inb_p(a19x_MEMR) & MII_READ_DATA_MASK)
	{
		// TRY AGAIN!
		// some kind of PHY need to do this twice...
		MiiOut3State
		if (inb_p(a19x_MEMR) & MII_READ_DATA_MASK)
			return FAIL;
	}
	for (i = 0; i < uBitsOfUshort; i++)
	{
		outb_p(MII_READ, a19x_MEMR);
		outb_p(MII_READ | MII_CLK, a19x_MEMR);

		*puRegData = (*puRegData << 1) |(USHORT)((inb_p(a19x_MEMR) >> MII_MDI_BIT_POSITION) & 0x0001);
	}
	MiiOut3State

	*puRegData &= ~PhyRegsReservedBitsMasks[uRegNum];

	return SUCCESS;
}

//***************************************************************************
// Write the value to the MII register
//***************************************************************************
void
MiiWriteRegister(
                struct net_device *dev,
		USHORT uPhyAddr,
		USHORT uRegNum,
		USHORT uRegData
		)
{

        long e8390_base = dev->base_addr;

	ULONG	ulCommand = ((ULONG)uPhyAddr <<PHY_ADDR_ALIGN)
			|((ULONG)uRegNum <<REG_ADDR_ALIGN)
			|MII_WRITE_FRAME
			|(ULONG)(uRegData & ~PhyRegsReservedBitsMasks[uRegNum]);
	USHORT	uBitsOfUshort = sizeof(USHORT) * 8;

	A19xWriteMii(dev,0, 16);
	A19xWriteMii(dev,PRE, (USHORT)(2*uBitsOfUshort));
	A19xWriteMii(dev,ulCommand, (USHORT)(2*uBitsOfUshort));
	MiiOut3State
}

