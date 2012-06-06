/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <gj@denx.de>
 * David Mueller <d.mueller@elsoft.ch>
 *
 * Configuation settings for the SAMSUNG SMDK6400(mDirac-III) board.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_S5PC100		1		/* in a SAMSUNG S3C6410 SoC */
#define CONFIG_S5PC1XX		1		/* in a SAMSUNG S3C64XX Family  */
#define CONFIG_SMDKC100		1

#define MEMORY_SIZE_256

//#define CONFIG_S5PC100_EVT1
#define CONFIG_S5PC100_EVT2

#define BOOT_ONENAND		0x1
#define BOOT_NAND		0x2
#define BOOT_MMCSD		0x3

#define MEMORY_BASE_ADDRESS	0x20000000

/* IROM specific data */
#define SDMMC_BLK_SIZE        (0xd0020230)

/* MMC configuration */
#if defined(CONFIG_S5PC100_EVT2)
#define CONFIG_MEMORY_UPPER_CODE
#define CONFIG_GENERIC_MMC
#define CONFIG_S3C_HSMMC
#undef DEBUG_S3C_HSMMC

/* The macro for MMC channel 0 is defined by default and can't be undefined */
#define USE_MMC0
#define MMC_MAX_CHANNEL		3
#endif

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000	/* the SMDK6400 has 12MHz input clock */

#define CONFIG_ENABLE_MMU

#ifdef CONFIG_ENABLE_MMU
#define virt_to_phys(x)	virt_to_phy_smdkc100(x)
#else
#define virt_to_phys(x)	(x)
#endif

#define CONFIG_MEMORY_UPPER_CODE
//#undef	CONFIG_MEMORY_UPPER_CODE

#undef CONFIG_USE_IRQ				/* we don't need IRQ/FIQ stuff */

#define CONFIG_INCLUDE_TEST

#define CONFIG_ZIMAGE_BOOT
#define CONFIG_IMAGE_BOOT

#define BOARD_LATE_INIT

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG

/*
 * Architecture magic and machine type
 */
#define MACH_TYPE		1826
#define UBOOT_MAGIC		(0x43090000 | MACH_TYPE)

/* Power Management is enabled */
#define CONFIG_PM

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#undef CONFIG_SKIP_RELOCATE_UBOOT
#undef CONFIG_USE_NOR_BOOT

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 1024*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

#define CFG_STACK_SIZE		512*1024

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_SMC911X	1	/* we have a SMC9115 on-board */

#ifdef 	CONFIG_DRIVER_SMC911X
#define CONFIG_DRIVER_SMC911X_16_BIT
#undef	CONFIG_DRIVER_CS8900
#define CONFIG_DRIVER_SMC911X_BASE	0x98800300
#else
#define CONFIG_DRIVER_CS8900	1	/* we have a CS8900 on-board */
#define CS8900_BASE	  	0x18800300
#define CS8900_BUS16		1 	/* the Linux driver does accesses as shorts */
#endif

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1          1	/* we use SERIAL 1 on SMDK6400 */

#define CFG_HUSH_PARSER			/* use "hush" command parser	*/
#ifdef CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#endif

#define CONFIG_CMDLINE_EDITING

#undef CONFIG_S3C64XX_I2C		/* this board has H/W I2C */
#ifdef CONFIG_S3C64XX_I2C
#define CONFIG_HARD_I2C		1
#define CFG_I2C_SPEED		50000
#define CFG_I2C_SLAVE		0xFE
#endif

#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT

#define CONFIG_USB_OHCI
#undef CONFIG_USB_STORAGE
#define CONFIG_S3C_USBD

#define USBD_DOWN_ADDR		0xc0000000

/************************************************************
 * RTC
 ************************************************************/
/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200

/***********************************************************
 * Command definition
 ***********************************************************/
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_USB
#define CONFIG_CMD_REGINFO
#define	CONFIG_CMD_NAND
#define CONFIG_CMD_ONENAND
#define CONFIG_CMD_MOVINAND
#define CONFIG_CMD_PING
#define CONFIG_CMD_DATE

#include <config_cmd_default.h>

#define CONFIG_CMD_ELF
#define CONFIG_CMD_DHCP
//#define CONFIG_CMD_I2C

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH

/*#define CONFIG_BOOTARGS    	"root=ramfs devfs=mount console=ttySA0,9600" */
#define CONFIG_ETHADDR		00:40:5c:26:0a:5b
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		192.168.0.20
#define CONFIG_SERVERIP		192.168.0.10
#define CONFIG_GATEWAYIP	192.168.0.1

#define CONFIG_ZERO_BOOTDELAY_CHECK

#define CONFIG_NET_MULTI
#undef	CONFIG_NET_MULTI

#ifdef CONFIG_NET_MULTI
#define CONFIG_DRIVER_SMC911X_BASE	0x98800300
#define CONFIG_DRIVER_SMC911X_16_BIT
#define CONFIG_DRIVER_CS8900
#define CS8900_BASE	  		0x18800300
#define CS8900_BUS16
#else
#define CONFIG_DRIVER_SMC911X_16_BIT
#define CONFIG_DRIVER_SMC911X_BASE	0x98800300
#undef	CONFIG_DRIVER_CS8900
#endif

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory		*/
#define CFG_PROMPT		"SMDKC100 # "	/* Monitor Command Prompt	*/
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE		384		/* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	MEMORY_BASE_ADDRESS	/* memtest works on	*/
#if defined(MEMORY_SIZE_256)
#define CFG_MEMTEST_END		MEMORY_BASE_ADDRESS + 0xfe00000		/* 256 MB in DRAM	*/
#else
#define CFG_MEMTEST_END		MEMORY_BASE_ADDRESS + 0x7e00000		/* 256 MB in DRAM	*/
#endif

#undef CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR		MEMORY_BASE_ADDRESS	/* default load address	*/

/* the PWM TImer 4 uses a counter of 15625 for 10 ms, so we need */
/* it to wrap 100 times (total 1562500) to get 1 sec. */
#define CFG_HZ			1562500		// at PCLK 50MHz

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	0x40000		/* regular stack 256KB */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

#define CONFIG_CLK_666_166_66
//#define CONFIG_CLK_833_166_66

#if defined(CONFIG_CLK_666_166_66)
#define APLL_MDIV       333
#define APLL_PDIV       3
#define APLL_SDIV       1
#elif defined(CONFIG_CLK_833_166_66)
#define APLL_MDIV       417
#define APLL_PDIV       3
#define APLL_SDIV       1
#endif

#define MPLL_MDIV	89
#define MPLL_PDIV	2
#define MPLL_SDIV	1
#define EPLL_MDIV	135
#define EPLL_PDIV	3
#define EPLL_SDIV	3
#define HPLL_MDIV	96
#define HPLL_PDIV	6
#define HPLL_SDIV	3

/* CLK_DIV0 */
#define APLL_RATIO	0
#define ARM_RATIO	4
#define D0_BUS_RATIO	8
#define PCLKD0_RATIO	12
#define SECSS_RATIO	16
#define CLK_DIV0_MASK	0x3fff
#define set_pll(mdiv, pdiv, sdiv)	(1<<31 | mdiv<<16 | pdiv<<8 | sdiv)

#define APLL_VAL	set_pll(APLL_MDIV,APLL_PDIV,APLL_SDIV)
#define MPLL_VAL	set_pll(MPLL_MDIV,MPLL_PDIV,MPLL_SDIV)
#define EPLL_VAL	set_pll(EPLL_MDIV,EPLL_PDIV,EPLL_SDIV)
#define HPLL_VAL	set_pll(HPLL_MDIV,HPLL_PDIV,HPLL_SDIV)

#if defined(CONFIG_CLK_666_166_66)
#define CLK_DIV0_VAL    ((0<<APLL_RATIO)|(0<<ARM_RATIO)|(3<<D0_BUS_RATIO)|(1<<PCLKD0_RATIO)|(1<<SECSS_RATIO))
#elif defined(CONFIG_CLK_833_166_66)
#define CLK_DIV0_VAL    ((0<<APLL_RATIO)|(0<<ARM_RATIO)|(4<<D0_BUS_RATIO)|(1<<PCLKD0_RATIO)|(1<<SECSS_RATIO))
#endif

#define CLK_DIV1_VAL	((1<<16)|(1<<12)|(1<<8)|(1<<4))
#define CLK_DIV2_VAL	(1<<0)

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */


#define CONFIG_NR_DRAM_BANKS	1	   /* we have 2 bank of DRAM */
#define PHYS_SDRAM_1		MEMORY_BASE_ADDRESS /* SDRAM Bank #1 */
#if defined(MEMORY_SIZE_256)
#define PHYS_SDRAM_1_SIZE	0x10000000 /* 256 MB */
#else
#define PHYS_SDRAM_1_SIZE	0x08000000 /* 128 MB */
#endif

#define CFG_FLASH_BASE		0x00000000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	0	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	1024
#define CONFIG_AMD_LV800
#define PHYS_FLASH_SIZE		0x100000

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(5*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(5*CFG_HZ) /* Timeout for Flash Write */

#define CFG_ENV_ADDR		0
#define CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector */

/*
 * SMDKC100 board specific data
 */

#define CONFIG_IDENT_STRING	" for SMDKC100"

/* total memory required by uboot */
#define CFG_UBOOT_SIZE		(2*1024*1024)

#if defined(MEMORY_SIZE_256)
/* base address for uboot */
#ifdef CONFIG_ENABLE_MMU
#define CFG_UBOOT_BASE		0xcfe00000
#else
#define CFG_UBOOT_BASE		0x2fe00000
#endif
#else
/* base address for uboot */
#ifdef CONFIG_ENABLE_MMU
#define CFG_UBOOT_BASE		0xc7e00000
#else
#define CFG_UBOOT_BASE		0x27e00000
#endif
#endif

#if defined(MEMORY_SIZE_256)
#define CFG_PHY_UBOOT_BASE	MEMORY_BASE_ADDRESS + 0xfe00000
#else
#define CFG_PHY_UBOOT_BASE	MEMORY_BASE_ADDRESS + 0x7e00000
#endif

#define CFG_ENV_OFFSET		0x0007C000

/* nand copy size from nand to DRAM.*/
#define	COPY_BL2_SIZE		0x80000

/* NAND configuration */
#define CFG_MAX_NAND_DEVICE     1
#define CFG_NAND_BASE           (0xE7200000)
#define NAND_MAX_CHIPS          1

#define NAND_DISABLE_CE()	(NFCONT_REG |= (1 << 1))
#define NAND_ENABLE_CE()	(NFCONT_REG &= ~(1 << 1))
#define NF_TRANSRnB()		do { while(!(NFSTAT_REG & (1 << 0))); } while(0)

#define CFG_NAND_SKIP_BAD_DOT_I	1  /* ".i" read skips bad blocks   */
#define	CFG_NAND_WP		1
#define CFG_NAND_YAFFS_WRITE	1  /* support yaffs write */

#define CFG_NAND_HWECC
#define CONFIG_NAND_BL1_8BIT_ECC
#undef	CFG_NAND_FLASH_BBT

/*
 * SD/MMC detection takes a little long time
 * So, you can ignore detection process for SD/MMC card
 */
#undef	CONFIG_NO_SDMMC_DETECTION

#define CONFIG_MTDPARTITION	"40000 3c0000 3000000"
#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTCOMMAND	"onenand read c0008000 40000 3c0000;bootm c0008000"
/* OneNAND configuration */
#define CFG_ONENAND_BASE 	(0xe7100000)
#define CFG_MAX_ONENAND_DEVICE	1

#define CONFIG_BOOT_ONENAND_IROM
#define CONFIG_NAND
#define CONFIG_BOOT_NAND
#define CONFIG_ONENAND
#define ONENAND_REG_DBS_DFS_WIDTH (0x160)
#define ONENAND_REG_FLASH_AUX_CNTRL       (0x300)

#define GPNCON_OFFSET		0x830
#define GPNDAT_OFFSET		0x834
#define GPNPUD_OFFSET		0x838

#define CFG_ENV_IS_IN_AUTO

#endif	/* __CONFIG_H */
