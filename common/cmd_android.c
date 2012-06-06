/*
 * common/cmd_android.c
 *
 *  $Id: cmd_android.c,v 1.2 2009/11/21 00:11:42 dark0351 Exp $
 *
 * (C) Copyright 2009
 * Byungjae Lee, Samsung Erectronics, bjlee@samsung.com.
 *	- support for S3C2412, S3C2443, S3C6400, S5PC100 and S5PC110
 *
 * (C) Copyright SAMSUNG Electronics
 *      wc7.jang  <wc7.jang@samsung.com>
 *      - add Android Command fo S5PC100 and S5PC110
 *
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

/*
 * Android Installation Functions
 *
 */

#include <common.h>
#include <command.h>
#include <cmd_onenand.h>

#include <regs.h>

#if defined(CONFIG_CMD_ONENAND)

#if defined(CONFIG_S3C2412) || defined(CONFIG_S3C2442)
#include "../cpu/s3c24xx/usbd-fs.h"
#elif defined(CONFIG_S3C2443) || defined(CONFIG_S3C2450) || defined(CONFIG_S3C2416)
#include "../cpu/s3c24xx/usbd-hs.h"
#elif defined(CONFIG_S3C6400) || defined(CONFIG_S3C6410) || defined(CONFIG_S3C6430)
#include "../cpu/s3c64xx/usbd-otg-hs.h"
#elif defined(CONFIG_S5PC100)
#include "../cpu/s5pc1xx/usbd-otg-hs.h"
#elif defined(CONFIG_S5PC110)
#include "../cpu/s5pc11x/usbd-otg-hs.h"
#elif defined(CONFIG_S5P6440)
#include "../cpu/s5p64xx/usbd-otg-hs.h"
#elif defined(CONFIG_S5P6442)
#include "../cpu/s5p644x/usbd-otg-hs.h"
#else
#error "* CFG_ERROR : you have to setup right Samsung CPU configuration"
#endif


#define ANDROID_UBOOT_ADDRESS		0x0
#define ANDROID_ZIMAGE_ADDRESS		0x600000
#define ANDROID_RAMDISK_ADDRESS		0xB00000
#define ANDROID_SYSTEM_ADDRESS		0xE00000
#define ANDROID_USERDATA_ADDRESS	0xB800000

#define ANDROID_OK	1
#define ANDROID_FAIL	0

#if defined(CONFIG_S5PC100)
#define ANDROID_DOWNLOAD_ADDRESS	0x20008000
#elif defined(CONFIG_S5PC110)
#define ANDROID_DOWNLOAD_ADDRESS	0x40000000
#elif defined(CONFIG_S3C6410)
#define ANDROID_DOWNLOAD_ADDRESS	0x50008000
#endif


#define	PRINTF(fmt,args...)	printf (fmt ,##args)


static const char pszMe[] = "usbd: ";

otg_dev_t android_usbd_download()
{
	otg_dev_t	nOtg;

	s3c_usbd_dn_addr = ANDROID_DOWNLOAD_ADDRESS;
	s3c_receive_done = 0;

	s3c_usbctl_init();
	s3c_usbc_activate();

	printf("Now, Waiting for Android image downloading.\n");
	printf("Press the 'USB Port => Download' button.\n");

	while (1) {
		if (S3C_USBD_DETECT_IRQ()) {
			s3c_udc_int_hndlr();
			S3C_USBD_CLEAR_IRQ();
		}

		if (s3c_receive_done)
			break;

		if (serial_tstc()) {
			serial_getc();
			break;
		}
	}
	
	nOtg.dn_addr 		= s3c_usbd_dn_addr;
	nOtg.dn_filesize 	= s3c_usbd_dn_cnt >> 18;
	s3c_usbd_dn_cnt 	= (nOtg.dn_filesize << 18) + 0x40000;
	nOtg.dn_filesize 	= s3c_usbd_dn_cnt;
	
	/* when operation is done, usbd must be stopped */
	s3c_usb_stop();

	return nOtg;
}


int do_insAndroid_Uboot()
{
	otg_dev_t	nOtg;
	size_t 		retlen = 0;

	printf("Start uboot image installation. \n");

	nOtg = android_usbd_download();
	onenand_block_erase(ANDROID_UBOOT_ADDRESS, nOtg.dn_filesize, 0);
	onenand_block_write(ANDROID_UBOOT_ADDRESS, nOtg.dn_filesize, &retlen, (u8 *)ANDROID_DOWNLOAD_ADDRESS);		

	printf("Completed uboot image (%x byte) installation. \n", nOtg.dn_filesize);
	
	return ANDROID_OK;
}

int do_insAndroid_zImage()
{
	otg_dev_t	nOtg;
	size_t 		retlen = 0;

	printf("Start kernel image installation. \n");

	nOtg = android_usbd_download();
	onenand_block_erase(ANDROID_ZIMAGE_ADDRESS, nOtg.dn_filesize, 0);
	onenand_block_write(ANDROID_ZIMAGE_ADDRESS, nOtg.dn_filesize, &retlen, (u8 *)ANDROID_DOWNLOAD_ADDRESS);		

	printf("Completed kernel image (%x byte) installation. \n", nOtg.dn_filesize);
	
	return ANDROID_OK;
}

int do_insAndroid_Ramdisk()
{
	otg_dev_t	nOtg;
	size_t 		retlen = 0;

	printf("Start ramdisk image installation. \n");

	nOtg = android_usbd_download();
	onenand_block_erase(ANDROID_RAMDISK_ADDRESS, nOtg.dn_filesize, 0);
	onenand_block_write(ANDROID_RAMDISK_ADDRESS, nOtg.dn_filesize, &retlen, (u8 *)ANDROID_DOWNLOAD_ADDRESS);		

	printf("Completed ramdisk image(%x byte) installation. \n", nOtg.dn_filesize);
	
	return ANDROID_OK;
}

int do_insAndroid_System()
{
	otg_dev_t	nOtg;
	size_t 		retlen = 0;

	printf("Start system image installation. \n");

	nOtg = android_usbd_download();
	onenand_block_erase(ANDROID_SYSTEM_ADDRESS, nOtg.dn_filesize, 0);
	onenand_write_yaffs2(ANDROID_SYSTEM_ADDRESS, nOtg.dn_filesize, &retlen, (u8 *)ANDROID_DOWNLOAD_ADDRESS);		

	printf("Completed system image(%x byte) installation. \n", nOtg.dn_filesize);
	
	return ANDROID_OK;
}

int do_insAndroid_Userdata()
{
	otg_dev_t	nOtg;
	size_t 		retlen = 0;

	printf("Start userdata image installation. \n");

	nOtg = android_usbd_download();
	onenand_block_erase(ANDROID_USERDATA_ADDRESS, nOtg.dn_filesize, 0);
	onenand_write_yaffs2(ANDROID_USERDATA_ADDRESS, nOtg.dn_filesize, &retlen, (u8 *)ANDROID_DOWNLOAD_ADDRESS);		

	printf("Completed userdata image(%x byte) installation. \n", nOtg.dn_filesize);
	
	return ANDROID_OK;
}


int do_install_android( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *cmd;
	cmd = argv[1];
	
	if(strcmp(cmd, "uboot") == 0){
		return do_insAndroid_Uboot();
	}
	else if(strcmp(cmd, "zimage") == 0){
		return do_insAndroid_zImage();
	}
	else if(strcmp(cmd, "ramdisk") == 0){
		return do_insAndroid_Ramdisk();
	}
	else if(strcmp(cmd, "system") == 0){
		return do_insAndroid_System();
	}
	else if(strcmp(cmd, "userdata") == 0){
		return do_insAndroid_Userdata();
	}
				
	return 1;
}


U_BOOT_CMD(
	insdroid, 2, 0, do_install_android,
	"insdroid\t 	   - Android sub-system\n",
	"uboot    - Install u-boot image for Android\n"
	"insdroid zimage   - Install zimage image for Android\n"
	"insdroid ramdisk  - Install ramdisk image for Android\n"
	"insdroid system   - Install system image for Android\n"		
	"insdroid userdata - Install userdata image for Android\n"
	"movi write u-boot {address} - Make bootable SD card with uboot\n"
);

#endif // CONFIG_CMD_ONENAND

