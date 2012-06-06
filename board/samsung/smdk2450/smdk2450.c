/*
 * (C) Copyright 2007-2008
 * Jong - pill, Lee <boyko.lee@samsung.com>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
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

#include <common.h>
#include <regs.h>
//#include <linux/mtd/onenand_regs.h>

//#include <asm/io.h>

/* ------------------------------------------------------------------------- */
static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n" "bne 1b":"=r" (loops):"0"(loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

static void cs8900_pre_init(void)
{
#if 1
	SMBIDCYR1_REG = 0;			//Bank1 Idle cycle ctrl.
	SMBWSTWRR1_REG = 14;			//Bank1 Write Wait State ctrl.
	SMBWSTOENR1_REG = 2;			//Bank1 Output Enable Assertion Delay ctrl.     Tcho?
	SMBWSTWENR1_REG = 2;			//Bank1 Write Enable Assertion Delay ctrl.
	SMBWSTRDR1_REG = 14;			//Bank1 Read Wait State cont. = 14 clk	    Tacc?
#endif
	SMBCR1_REG |=  ((1<<15)|(1<<7));		//dsf
	SMBCR1_REG |=  ((1<<2)|(1<<0));		//SMWAIT active High, Read Byte Lane Enabl	    WS1?
	SMBCR1_REG &= ~((3<<20)|(3<<12));	//SMADDRVALID = always High when Read/Write
	SMBCR1_REG &= ~(3<<4);			//Clear Memory Width
	SMBCR1_REG |=  (1<<4);			//Memory Width = 16bit
}

static void usb_pre_init (void)
{
	CLKDIV1CON_REG |= 1<<4;

	USB_RSTCON_REG = 0x1;
	delay(500);
	USB_RSTCON_REG = 0x2;
	delay(500);
	USB_RSTCON_REG = 0x0;
	delay(500);

//	USB_PHYCTRL_REG &= ~0x2;
	USB_CLKCON_REG |= 0x2;
}

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	cs8900_pre_init();
	usb_pre_init();

//	SMBCR4_REG = 0x00000015;
//	SMBCR1_REG = 0x00000015;
	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = (PHYS_SDRAM_1+0x100);

#if 0
	icache_enable();
	dcache_enable();
#endif
	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

#ifdef BOARD_LATE_INIT
int board_late_init (void)
{
	uint *magic = (uint*)(PHYS_SDRAM_1);
	char boot_cmd[100];

	if ((0x24564236 == magic[0]) && (0x20764316 == magic[1])) {
		sprintf(boot_cmd, "nand erase 0 40000;nand write %08x 0 40000", PHYS_SDRAM_1 + 0x8000);
		magic[0] = 0;
		magic[1] = 0;
		printf("\nready for self-burning U-Boot image\n\n");
		setenv("bootdelay", "0");
		setenv("bootcmd", boot_cmd);
	}

	return 0;
}
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	vu_long *mem_reg = (vu_long*) 0x48000000;

	printf("Board: SMDK2450 ");
	switch ((*mem_reg>>1) & 0x7) {
	case 0:
		puts("SDRAM\n");
		break;
	case 1:
		puts("DDR2\n");
		break;
	case 2:
		puts("Mobile SDRAM\n");
		break;
	case 4:
		puts("DDR\n");
		break;
	case 6:
		puts("Mobile DDR\n");
		break;
	default:
		puts("unknown Memory Type\n");
	}
	return (0);
}
#endif

#ifdef CONFIG_ENABLE_MMU
ulong virt_to_phy_smdk2450(ulong addr)
{
	if ((0xC0000000 <= addr) && (addr < 0xC4000000))
		return (addr - 0xC0000000 + 0x30000000);
	else
		printf("do not support this address : %08lx\n", addr);

	return addr;
}
#endif

#if defined(CONFIG_CMD_NAND) && defined(CFG_NAND_LEGACY)
#include <linux/mtd/nand.h>
extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];
void nand_init(void)
{
	nand_probe(CFG_NAND_BASE);
	if (nand_dev_desc[0].ChipID != NAND_ChipID_UNKNOWN) {
		print_size(nand_dev_desc[0].totlen, "\n");
	}
}
#endif
