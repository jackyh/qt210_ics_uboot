/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
#include <asm/io.h>


/* ------------------------------------------------------------------------- */
#define SMSC9220_Tacs	(0x0)	// 0clk		address set-up
#define SMSC9220_Tcos	(0x4)	// 4clk		chip selection set-up
#define SMSC9220_Tacc	(0xe)	// 14clk	access cycle
#define SMSC9220_Tcoh	(0x1)	// 1clk		chip selection hold
#define SMSC9220_Tah	(0x4)	// 4clk		address holding time
#define SMSC9220_Tacp	(0x6)	// 6clk		page mode access cycle
#define SMSC9220_PMC	(0x0)	// normal(1data)page mode configuration

#define SROM_DATA16_WIDTH(x)	(1<<((x*4)+0))
#define SROM_ADDR_MODE_16BIT(x)	(1<<((x*4)+1))
#define SROM_WAIT_ENABLE(x)	(1<<((x*4)+1))
#define SROM_BYTE_ENABLE(x)	(1<<((x*4)+2))

/* ------------------------------------------------------------------------- */
#define DM9000_Tacs	(0x0)	// 0clk		address set-up
#define DM9000_Tcos	(0x4)	// 4clk		chip selection set-up
#define DM9000_Tacc	(0xE)	// 14clk	access cycle
#define DM9000_Tcoh	(0x1)	// 1clk		chip selection hold
#define DM9000_Tah	(0x4)	// 4clk		address holding time
#define DM9000_Tacp	(0x6)	// 6clk		page mode access cycle
#define DM9000_PMC	(0x0)	// normal(1data)page mode configuration


static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n" "bne 1b":"=r" (loops):"0"(loops));
}

/*
 * Miscellaneous platform dependent initialisations
 */

static void dm9000_pre_init(void)
{
	unsigned int tmp;

#if defined(DM9000_16BIT_DATA)
	SROM_BW_REG &= ~(0xf << 20);
	SROM_BW_REG |= (0<<23) | (0<<22) | (0<<21) | (1<<20);

#else	
	SROM_BW_REG &= ~(0xf << 20);
	SROM_BW_REG |= (0<<19) | (0<<18) | (0<<16);
#endif
	SROM_BC5_REG = ((0<<28)|(1<<24)|(5<<16)|(1<<12)|(4<<8)|(6<<4)|(0<<0));

	tmp = MP01CON_REG;
	tmp &=~(0xf<<20);
	tmp |=(2<<20);
	MP01CON_REG = tmp;
}

static void smsc9220_pre_init(int bank_num)
{
	unsigned int tmp;
//	unsigned char smc_bank_num=1;

	/* gpio configuration */
	tmp = MP01CON_REG;
	tmp &= ~(0xf << bank_num*4);
	tmp |= (0x2 << bank_num*4);
	MP01CON_REG = tmp;

	tmp = SROM_BW_REG;
	tmp &= ~(0xF<<(bank_num * 4));
	tmp |= SROM_DATA16_WIDTH(bank_num);
	tmp |= SROM_ADDR_MODE_16BIT(bank_num);
	SROM_BW_REG = tmp;

	if(bank_num == 0)
		SROM_BC0_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
	else if(bank_num == 1)
		SROM_BC1_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
	else if(bank_num == 2)
		SROM_BC2_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
	else if(bank_num == 3)
		SROM_BC3_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
	else if(bank_num == 4)
		SROM_BC3_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
	else if(bank_num == 5)
		SROM_BC3_REG = ((SMSC9220_Tacs<<28)|(SMSC9220_Tcos<<24)|(SMSC9220_Tacc<<16)|(SMSC9220_Tcoh<<12)|(SMSC9220_Tah<<8)|(SMSC9220_Tacp<<4)|(SMSC9220_PMC));
}

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
#ifdef CONFIG_DRIVER_SMC911X
	smsc9220_pre_init(5);
#endif

#ifdef CONFIG_DRIVER_DM9000
	dm9000_pre_init();
#endif

	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = (PHYS_SDRAM_1+0x100);

	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

#if defined(PHYS_SDRAM_2)
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
#endif

#if defined(PHYS_SDRAM_3)
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;
#endif

	return 0;
}

#ifdef BOARD_LATE_INIT
#if defined(CONFIG_BOOT_NAND)
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
#elif defined(CONFIG_BOOT_MOVINAND)
int board_late_init (void)
{
	uint *magic = (uint*)(PHYS_SDRAM_1);
	char boot_cmd[100];
	int hc;

	hc = (magic[2] & 0x1) ? 1 : 0;

	if ((0x24564236 == magic[0]) && (0x20764316 == magic[1])) {
		sprintf(boot_cmd, "movi init %d %d;movi write u-boot %08x", magic[3], hc, PHYS_SDRAM_1 + 0x8000);
		magic[0] = 0;
		magic[1] = 0;
		printf("\nready for self-burning U-Boot image\n\n");
		setenv("bootdelay", "0");
		setenv("bootcmd", boot_cmd);
	}

	return 0;
}
#else
int board_late_init (void)
{
	return 0;
}
#endif
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
#ifdef CONFIG_MCP_SINGLE
#if defined(CONFIG_VOGUES)
	printf("\nBoard:   VOGUESV210\n");
#else
	printf("\nBoard:   SMDKV210\n");
#endif //CONFIG_VOGUES
#else
	printf("\nBoard:   SMDKC110\n");
#endif
	return (0);
}
#endif

#ifdef CONFIG_ENABLE_MMU

#ifdef CONFIG_MCP_SINGLE
ulong virt_to_phy_smdkc110(ulong addr)
{
	if ((0xc0000000 <= addr) && (addr < 0xd0000000))
		return (addr - 0xc0000000 + 0x20000000);
	else
		printf("The input address don't need "\
			"a virtual-to-physical translation : %08lx\n", addr);

	return addr;
}
#else
ulong virt_to_phy_smdkc110(ulong addr)
{
	if ((0xc0000000 <= addr) && (addr < 0xd0000000))
		return (addr - 0xc0000000 + 0x30000000);
	else if ((0x30000000 <= addr) && (addr < 0x50000000))
		return addr;
	else
		printf("The input address don't need "\
			"a virtual-to-physical translation : %08lx\n", addr);

	return addr;
}
#endif

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
