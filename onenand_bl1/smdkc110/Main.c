/*
 * Copyright (c) 2009 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * Main: Copy 2 U-Boot from OneNAND to DRAM 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/* This code support ONLY 4KB OneNAND!!!!! - djpark */

#include <linux/mtd/onenand_regs.h>

#define ONENAND_BASE		0xB0000000

#ifdef S5PV210
#define ONENAND_PAGE_SIZE	2048
#define UBOOT_PHY_BASE		0x23E00000 /* For V210 */
#else
#define ONENAND_PAGE_SIZE	4096
#define UBOOT_PHY_BASE		0x33E00000 /* For C110 */
#endif

#define READ_INTERRUPT()	\
	onenand_readw(ONENAND_BASE + ONENAND_REG_INTERRUPT)

#define onenand_block_address(block)		(block)
#define onenand_sector_address(page)		(page << ONENAND_FPA_SHIFT)
#define onenand_buffer_address()			((1 << 3) << 8)
//#define onenand_buffer_address()			((0 << 3) << 8)	
#define onenand_bufferram_address(block)	(0)

inline unsigned short onenand_readw (unsigned int addr)
{
	return *(unsigned short*)addr;
}

inline void onenand_writew (unsigned short value, unsigned int addr)
{
	*(unsigned short*)addr = value;
}

void(*run_uboot)(void) = UBOOT_PHY_BASE;

//void ONENAND_WriteReg()
//
void ONENAND_ReadPage(
	void* base, unsigned int block, unsigned int page)
{
	int len;
	unsigned short* dest;
	unsigned short* src;

	// Block Number
	onenand_writew(onenand_block_address(block),
			ONENAND_BASE + ONENAND_REG_START_ADDRESS1);

	// BufferRAM
	onenand_writew(onenand_bufferram_address(block),
			ONENAND_BASE + ONENAND_REG_START_ADDRESS2);

	// Page (Sector) Number Set: FPA, FSA
	onenand_writew(onenand_sector_address(page),
			ONENAND_BASE + ONENAND_REG_START_ADDRESS8);

	// BSA, BSC
	onenand_writew(onenand_buffer_address(),
			ONENAND_BASE + ONENAND_REG_START_BUFFER);

	// Interrupt clear
	onenand_writew(ONENAND_INT_CLEAR, ONENAND_BASE + ONENAND_REG_INTERRUPT);

	onenand_writew(ONENAND_CMD_READ, ONENAND_BASE + ONENAND_REG_COMMAND);

#if 1
	while (!(READ_INTERRUPT() & ONENAND_INT_READ))
		continue;
#else
	while (!(READ_INTERRUPT() & ONENAND_INT_MASTER))
		continue;
#endif

	len = ONENAND_PAGE_SIZE >> 1;
	dest = (unsigned short *)(base);
	src = (unsigned short *)(ONENAND_BASE + ONENAND_DATARAM);
	while (len-- > 0)
	{
		*dest++ = *src++;
	}
}

/*
 * Copy U-Boot from OneNAND to DRAM (368KB)
 */
void Main(void)
{
	volatile unsigned int base = UBOOT_PHY_BASE;
	int block, page;

	if(ONENAND_PAGE_SIZE == 4096) {
	#if defined(S5PC110_EVT1)
		page = 2;
	#else
		page = 4;
	#endif

		for (block=0; page < 64; page++) {
			ONENAND_ReadPage((void *)base, block, page);
			base += ONENAND_PAGE_SIZE;
		}

		for (block=1, page = 0; page < 32; page++) {
			ONENAND_ReadPage((void *)base, block, page);
			base += ONENAND_PAGE_SIZE;
		}
	} else {
		page = 4;

		for (block=0; page < 64; page++) {
			ONENAND_ReadPage((void *)base, block, page);
			base += ONENAND_PAGE_SIZE;
		}

		for (block=1, page = 0; page < 64; page++) {
			ONENAND_ReadPage((void *)base, block, page);
			base += ONENAND_PAGE_SIZE;
		}
	}

	run_uboot();
}

