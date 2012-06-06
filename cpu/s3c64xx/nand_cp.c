/*
 * $Id: nand_cp.c,v 1.4 2008/04/08 00:41:40 jsgood Exp $
 *
 * (C) Copyright 2006 Samsung Electronics
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * You must make sure that all functions in this file are designed
 * to load only U-Boot image.
 *
 * So, DO NOT USE in common read.
 *
 * By scsuh.
 */


#include <common.h>
#include <asm/io.h>
#include <linux/mtd/nand.h>
#include <regs.h>

#define NAND_CONTROL_ENABLE()	(NFCONT_REG |= (1 << 0))

#define NF8_ReadPage(a,b,c)	(((int(*)(uint, uint, uchar *))(*((uint *)(0x0c004000))))(a,b,c))
#define NF8_ReadPage_Adv(a,b,c)	(((int(*)(uint, uint, uchar *))(*((uint *)(0x0c004004))))(a,b,c))

void nand_bl2_copy(void)
{
	int block, page;
	volatile uint *base = (uint *) 0x57e00000;

	for (block = 0; block < 32; block++) {
		for (page = 0; page < 32; page++) {
			NF8_ReadPage(block, page, (uchar *)base);
			base += (512 / (sizeof(uint)));
		}
	}
}

void nand_bl2_copy_adv(int b4Kpage)
{
	int block, page;
	volatile uint *base = (uint *) 0x57e00000;

	if(b4Kpage == 1) {
		for (block = 0; block < 2; block++) {
			for (page = 0; page < 128; page++) {
				NF8_ReadPage_Adv(block, page, (uchar *) base);
				base += (4096 / (sizeof(uint)));
			}
		}
	} else {
		for (block = 0; block < 4; block++) {
			for (page = 0; page < 64; page++) {
				NF8_ReadPage_Adv(block, page, (uchar *) base);
				base += (2048 / (sizeof(uint)));
			}
		}
	}
}

/*
 * address format
 *              17 16         9 8            0
 * --------------------------------------------
 * | block(12bit) | page(5bit) | offset(9bit) |
 * --------------------------------------------
 */

static int nandll_read_page (uchar *buf, ulong addr, int large_block, int page_size)
{
        int i;

        NAND_ENABLE_CE();

        NFCMD_REG = NAND_CMD_READ0;

        /* Write Address */
        NFADDR_REG = 0;

	if (large_block)
	        NFADDR_REG = 0;

	NFADDR_REG = (addr) & 0xff;
	NFADDR_REG = (addr >> 8) & 0xff;
	NFADDR_REG = (addr >> 16) & 0xff;

	if (large_block)
		NFCMD_REG = NAND_CMD_READSTART;

        NF_TRANSRnB();

	/* for compatibility(2460). u32 cannot be used. by scsuh */
	for(i=0; i < page_size; i++) {
                *buf++ = NFDATA8_REG;
        }

        NAND_DISABLE_CE();
        return 0;
}

/*
 * Read data from NAND.
 */
static int nandll_read_blocks (ulong dst_addr, ulong size, int large_block, int page_shift)
{
        uchar *buf = (uchar *)dst_addr;
        int i;

        /* Read pages */
        for (i = 0; i < (size>>page_shift); i++, buf+=(1<<page_shift)) {
                nandll_read_page(buf, i, large_block, 1 << page_shift);
        }

        return 0;
}

int copy_uboot_to_ram (void)
{
	int large_block = 0;
	int i;
	vu_char id;
	int page_shift = 9;
	
	NAND_CONTROL_ENABLE();
        NAND_ENABLE_CE();
        NFCMD_REG = NAND_CMD_READID;
        NFADDR_REG =  0x00;

	/* wait for a while */
        for (i=0; i<200; i++);
	id = NFDATA8_REG;
	id = NFDATA8_REG;

	if (id > 0x80) {
		large_block = 1;
		id = NFDATA8_REG;
		id = NFDATA8_REG;
		switch (id & 3) {
			case 1:
				page_shift = 11; break;
			case 2: 
				page_shift = 12; break;
			case 3:
				page_shift = 13; break;
		}
	}

	/* read NAND Block.
	 * 128KB ->240KB because of U-Boot size increase. by scsuh
	 * So, read 0x3c000 bytes not 0x20000(128KB).
	 */
	return nandll_read_blocks(CFG_PHY_UBOOT_BASE, COPY_BL2_SIZE, large_block, page_shift);
}

