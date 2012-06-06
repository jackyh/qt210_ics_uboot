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
#define CONFIG_S5P6440		1		/* in a SAMSUNG S5P6440 SoC */
#define CONFIG_S5P64XX		1		/* in a SAMSUNG S5P64XX Family  */
#define CONFIG_SMDK6440		1		/* on a SAMSUNG SMDK6440 Board  */
#define SMDK6440_EVT0				/* using EVT0 Chip */
//#define SMDK6440_EVT1				/* using EVT1 Chip */

#define MEMORY_BASE_ADDRESS	(0x20000000)

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000	/* the SMDK6440 has 12MHz input clock */

#define CONFIG_ENABLE_MMU
#ifdef CONFIG_ENABLE_MMU
#define virt_to_phys(x)	virt_to_phy_smdk6440(x)
#else
#define virt_to_phys(x)	(x)
#endif

#define CONFIG_MEMORY_UPPER_CODE

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
#define MACH_TYPE		2032
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
 * select serial console configuration
 */
#define CONFIG_SERIAL2          	/* we use SERIAL 2 on SMDK6440 */

#define CFG_HUSH_PARSER			/* use "hush" command parser	*/
#ifdef CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#endif

#define CONFIG_CMDLINE_EDITING

#undef CONFIG_S5P64XX_I2C		/* this board has H/W I2C */
#ifdef CONFIG_S5P64XX_I2C
#define CONFIG_HARD_I2C		1
#define CFG_I2C_SPEED		50000
#define CFG_I2C_SLAVE		0xFE
#endif

#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT

#define CONFIG_USB_OHCI
#undef CONFIG_USB_STORAGE
#define CONFIG_S3C_USBD

#define USBD_DOWN_ADDR		(0xc0000000)

/************************************************************
 * RTC
 ************************************************************/
#define CONFIG_RTC_S5P64XX	1

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
//#define CONFIG_CMD_ONENAND
#define CONFIG_CMD_MOVINAND
#define CONFIG_CMD_PING
#define CONFIG_CMD_DATE
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_NET

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

#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTCOMMAND	"movi read kernel c0008000 ;movi read rootfs c0800000 C00000;bootm c0008000"
//#define CONFIG_BOOTCOMMAND	"nand read c0008000 80000 400000;nand read c0800000 400000 600000;bootm c0008000"
//#define CONFIG_BOOTARGS    	"root=ramfs devfs=mount console=ttySA0,9600"
#define CONFIG_ETHADDR		00:40:5c:26:0a:5b
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		192.168.0.20
#define CONFIG_SERVERIP		192.168.0.10
#define CONFIG_GATEWAYIP	192.168.0.1

#define CONFIG_ZERO_BOOTDELAY_CHECK

//#define CONFIG_NET_MULTI

#ifdef CONFIG_NET_MULTI
#define CONFIG_DRIVER_SMC911X
#define CONFIG_DRIVER_SMC911X_BASE	(0x18800300)
#define CONFIG_DRIVER_SMC911X_16_BIT
#define CONFIG_DRIVER_CS8900
#define CS8900_BASE	  		(0x18800300)
#define CS8900_BUS16

#else

#define CONFIG_DRIVER_DM9000	1

#ifdef 	CONFIG_DRIVER_SMC911X
#define CONFIG_DRIVER_SMC911X_16_BIT
#undef	CONFIG_DRIVER_CS8900
#define CONFIG_DRIVER_SMC911X_BASE	(0x18800300)
#endif

#ifdef CONFIG_DRIVER_CS8900
#define CS8900_BASE	  		(0x18800300)
#define CS8900_BUS16
#endif

#ifdef CONFIG_DRIVER_DM9000
#define CONFIG_DM9000_BASE		(0x80000000)
#define DM9000_IO			(CONFIG_DM9000_BASE)
#define DM9000_DATA			(CONFIG_DM9000_BASE+2)
#endif

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
#define CFG_PROMPT		"SMDK6440 # "	/* Monitor Command Prompt	*/
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE		384		/* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	(MEMORY_BASE_ADDRESS)	/* memtest works on	*/
#define CFG_MEMTEST_END		(MEMORY_BASE_ADDRESS + 0x7e00000)	/* 128 MB in DRAM */

#undef CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR		MEMORY_BASE_ADDRESS	/* default load address	*/

/* the PWM TImer 4 uses a counter of 41687 for 10 ms, so we need */
/* it to wrap 100 times (total 4168750) to get 1 sec. */
#define CFG_HZ			4168750		/* at PCLK 66MHz */

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

//#define CONFIG_CLK_667_166_83
#define CONFIG_CLK_533_133_66
//#define CONFIG_CLK_400_100_50

#define set_pll(mdiv, pdiv, sdiv)	(1<<31 | mdiv<<16 | pdiv<<8 | sdiv)

#if defined(CONFIG_CLK_667_166_83)
#define APLL_MDIV		667
#define APLL_PDIV		6
#define APLL_SDIV		1
#elif defined(CONFIG_CLK_533_133_66)
#define APLL_MDIV		533
#define APLL_PDIV		6
#define APLL_SDIV		1
#elif defined(CONFIG_CLK_400_100_50)
#define APLL_MDIV		400
#define APLL_PDIV		3
#define APLL_SDIV		2
#else
#error "Not Support Fequency or Mode!! you have to setup right configuration."
#endif

#define MPLL_MDIV		260
#define MPLL_PDIV		2
#define MPLL_SDIV		2

#define APLL_VAL	set_pll(APLL_MDIV, APLL_PDIV, APLL_SDIV)
#define Startup_APLL	(CONFIG_SYS_CLK_FREQ/(APLL_PDIV<<APLL_SDIV)*APLL_MDIV)

#define MPLL_VAL	set_pll(MPLL_MDIV, MPLL_PDIV, MPLL_SDIV)
#define Startup_MPLL	((CONFIG_SYS_CLK_FREQ)/(MPLL_PDIV<<MPLL_SDIV)*MPLL_MDIV)

#if defined(CONFIG_CLK_667_166_83)
#define	Startup_PCLKdiv		1
#define Startup_HCLKdiv		3
#define Startup_MPLLdiv		0
#define Startup_ARMdiv		0
#elif defined(CONFIG_CLK_533_133_66)
#define	Startup_PCLKdiv		1
#define Startup_HCLKdiv		3
#define Startup_MPLLdiv		0
#define Startup_ARMdiv		0
#elif defined(CONFIG_CLK_400_100_50)
#define	Startup_PCLKdiv		1
#define Startup_HCLKdiv		3
#define Startup_MPLLdiv		0
#define Startup_ARMdiv		0
#else
#error "Not Support Fequency or Mode!! you have to setup right configuration."
#endif

#define CLK_DIV_VAL	((Startup_PCLKdiv<<12)|(Startup_HCLKdiv<<8)|(Startup_MPLLdiv<<4)|Startup_ARMdiv)

#define Startup_HCLK	(Startup_APLL/(Startup_HCLKx2div+1)/(Startup_HCLKdiv+1))

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#ifndef CONFIG_SMDK6430_X5A

#define DMC1_MEM_CFG		0x00010012	/* Supports one CKE control, Chip1, Burst4, Row/Column bit */
#define DMC1_MEM_CFG2		0xB45
#define DMC1_CHIP0_CFG		0x150F8
#define DMC_DDR_32_CFG		0x0 		/* 32bit, DDR */

/* Memory Parameters */
/* DDR Parameters */
#define DDR_tREFRESH		7800		/* ns */
#define DDR_tRAS		45		/* ns (min: 45ns)*/
#define DDR_tRC 		68		/* ns (min: 67.5ns)*/
#define DDR_tRCD		23		/* ns (min: 22.5ns)*/
#define DDR_tRFC		80		/* ns (min: 80ns)*/
#define DDR_tRP 		23		/* ns (min: 22.5ns)*/
#define DDR_tRRD		15		/* ns (min: 15ns)*/
#define DDR_tWR 		15		/* ns (min: 15ns)*/
#define DDR_tXSR		120		/* ns (min: 120ns)*/
#define DDR_CASL		3		/* CAS Latency 3 */

#else

#define DMC1_MEM_CFG		0x00210011	/* Supports one CKE control, Chip1, Burst4, Row/Column bit */
#define DMC1_MEM_CFG2		0xB41
#define DMC1_CHIP0_CFG		0x150FC
#define DMC1_CHIP1_CFG		0x154FC
#define DMC_DDR_32_CFG		0x0		/* 32bit, DDR */

/* Memory Parameters */
/* DDR Parameters */
#define DDR_tREFRESH		5865		/* ns */
#define DDR_tRAS		50		/* ns (min: 45ns)*/
#define DDR_tRC 		68		/* ns (min: 67.5ns)*/
#define DDR_tRCD		23		/* ns (min: 22.5ns)*/
#define DDR_tRFC		133		/* ns (min: 80ns)*/
#define DDR_tRP 		23		/* ns (min: 22.5ns)*/
#define DDR_tRRD		20		/* ns (min: 15ns)*/
#define DDR_tWR 		20		/* ns (min: 15ns)*/
#define DDR_tXSR		125		/* ns (min: 120ns)*/
#define DDR_CASL		3		/* CAS Latency 3 */

#endif

/*
 * mDDR memory configuration
 */
#define SMDK6440_DDR		// define memory type[DDR] for revision cpu board. 
//#define SMDK6440_DDR2		// define memory type[DDR2] for revision cpu board.

#define DMC_DDR_BA_EMRS 	2
#define DMC_DDR_MEM_CASLAT	3
#define DMC_DDR_CAS_LATENCY	(DDR_CASL<<1)						//6   Set Cas Latency to 3
#define DMC_DDR_t_DQSS		1							// Min 0.75 ~ 1.25
#define DMC_DDR_t_MRD		2							//Min 2 tck
#define DMC_DDR_t_RAS		(((Startup_HCLK / 1000 * DDR_tRAS) - 1) / 1000000 + 1)	//7, Min 45ns
#define DMC_DDR_t_RC		(((Startup_HCLK / 1000 * DDR_tRC) - 1) / 1000000 + 1) 	//10, Min 67.5ns
#define DMC_DDR_t_RCD		(((Startup_HCLK / 1000 * DDR_tRCD) - 1) / 1000000 + 1) 	//4,5(TRM), Min 22.5ns
#define DMC_DDR_schedule_RCD	((DMC_DDR_t_RCD - 3) << 3)
#define DMC_DDR_t_RFC		(((Startup_HCLK / 1000 * DDR_tRFC) - 1) / 1000000 + 1) 	//11,18(TRM) Min 80ns
#define DMC_DDR_schedule_RFC	((DMC_DDR_t_RFC - 3) << 5)
#define DMC_DDR_t_RP		(((Startup_HCLK / 1000 * DDR_tRP) - 1) / 1000000 + 1) 	//4, 5(TRM) Min 22.5ns
#define DMC_DDR_schedule_RP	((DMC_DDR_t_RP - 3) << 3)
#define DMC_DDR_t_RRD		(((Startup_HCLK / 1000 * DDR_tRRD) - 1) / 1000000 + 1)	//3, Min 15ns
#define DMC_DDR_t_WR		(((Startup_HCLK / 1000 * DDR_tWR) - 1) / 1000000 + 1)	//Min 15ns
#define DMC_DDR_t_WTR		2
#define DMC_DDR_t_XP		2							//1tck + tIS(1.5ns)
#define DMC_DDR_t_XSR		(((Startup_HCLK / 1000 * DDR_tXSR) - 1) / 1000000 + 1)	//17, Min 120ns
#define DMC_DDR_t_ESR		DMC_DDR_t_XSR
#define DMC_DDR_REFRESH_PRD	(((Startup_HCLK / 1000 * DDR_tREFRESH) - 1) / 1000000) 	// TRM 2656
#define DMC_DDR_USER_CONFIG	1							// 2b01 : mDDR

#define CONFIG_NR_DRAM_BANKS	1	   /* we have 2 bank of DRAM */
#define PHYS_SDRAM_1		MEMORY_BASE_ADDRESS /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x10000000 /* 256 MB */

#define CFG_FLASH_BASE		(0x00000000)

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
 * SMDK6400 board specific data
 */

#define CONFIG_IDENT_STRING	"for SMDK6440"

/* total memory required by uboot */
#define CFG_UBOOT_SIZE		(2*1024*1024)

/* base address for uboot */
#ifdef CONFIG_ENABLE_MMU
#define CFG_UBOOT_BASE		(0xc7e00000)
#else
#define CFG_UBOOT_BASE		(0x27e00000)
#endif
#define CFG_PHY_UBOOT_BASE	(MEMORY_BASE_ADDRESS + 0x7e00000)

#define CFG_ENV_OFFSET		0x0007c000

/* nand copy size from nand to DRAM.*/
#define	COPY_BL2_SIZE		0x80000

#define CFG_MAX_NAND_DEVICE     1
#define CFG_NAND_BASE           (0x70200010)
#define CFG_NAND_SKIP_BAD_DOT_I	1  /* ".i" read skips bad blocks   */
#define	CFG_NAND_WP		1
#define CFG_NAND_YAFFS_WRITE	1  /* support yaffs write */
#define CFG_NAND_HWECC
#undef	CFG_NAND_FLASH_BBT

/* IROM specific data */
#ifdef SMDK6440_EVT0
#define SDMMC_BLK_SIZE        (0xd0021c54)
#endif
#ifdef SMDK6440_EVT1
#define SDMMC_BLK_SIZE        (0xd0021FF0)
#endif

/* SD/MMC configuration */
#define CONFIG_GENERIC_MMC
#define CONFIG_S3C_HSMMC
#undef DEBUG_S3C_HSMMC

/*
   Select a needed channel :
   It's recommanded to select only one channel to use 'movi' command
*/
#define USE_MMC0
#define MMC_MAX_CHANNEL		3
/* If OM_PIN is defined, USE_MMC0-2 macro will be ignored. */
#define OM_PIN			((GPNDAT_REG >> 13) & 7)

#ifdef OM_PIN
#define SDMMC_CHANNEL0	0x0
#define SDMMC_CHANNEL1	0x7
#endif

#define CONFIG_NAND_BL1_8BIT_ECC
#define NAND_MAX_CHIPS          1
#define NAND_DISABLE_CE()	(NFCONT_REG |= (1 << 1))
#define NAND_ENABLE_CE()	(NFCONT_REG &= ~(1 << 1))
#define NF_TRANSRnB()		do { while(!(NFSTAT_REG & (1 << 0))); } while(0)
#define CONFIG_MTDPARTITION	"40000 3c0000 3000000"
#define CFG_ONENAND_BASE 	(0x70100000)
#define CFG_MAX_ONENAND_DEVICE	1
#define CFG_ENV_IS_IN_AUTO

#endif	/* __CONFIG_H */

