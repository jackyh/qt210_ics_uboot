/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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
#include <asm/io.h>

ulong myflush (void);


#define FLASH_BANK_SIZE	PHYS_FLASH_SIZE

/* S29GL064N90TFI040 bottom boot block sectors(8MB)
 * 1st ~ 8th sectors	: 8KB
 * 9th ~ 135th sectors	: 64KB
 *
 * AM29LV800BB bottom boot block sectors(1MB)
 * 1st sector : 16KB
 * 2nd sector : 8KB
 * 3rd sector : 8KB
 * 4th sector : 32KB
 * 5th ~ 19th : 64KB
 */

#if defined(CONFIG_MX_LV640EB)
#define BOOT_SECT_SIZE		0x2000	/* 8 KB */
#define MAIN_SECT_SIZE  0x10000	/* 64 KB */
#elif defined(CONFIG_AMD_LV800)
#define FIRST_SECT_SIZE		0x4000	/* 16 KB */
#define SECOND_SECT_SIZE	0x2000	/* 8 KB */
#define THIRD_SECT_SIZE		0x2000	/* 8 KB */
#define FOURTH_SECT_SIZE	0x8000	/* 32 KB */
#define MAIN_SECT_SIZE		0x10000	/* 64 KB */
#endif


flash_info_t flash_info[CFG_MAX_FLASH_BANKS];


#define CMD_READ_ARRAY		0x0000F0F0
#define CMD_UNLOCK1		0x0000AAAA
#define CMD_UNLOCK2		0x00005555
#define CMD_ERASE_SETUP		0x00008080
#define CMD_ERASE_CONFIRM	0x00003030
#define CMD_ERASE_ALL		0x00001010
#define CMD_PROGRAM		0x0000A0A0
#define CMD_UNLOCK_BYPASS	0x00000020

#define MEM_FLASH_RESET		(*(volatile u16 *)(CFG_FLASH_BASE))
#define MEM_FLASH_ADDR1		(*(volatile u16 *)(CFG_FLASH_BASE + (0x00000555 << 1)))
#define MEM_FLASH_ADDR2		(*(volatile u16 *)(CFG_FLASH_BASE + (0x000002AA << 1)))

#define BIT_ERASE_DONE		0x00000080
#define BIT_RDY_MASK		0x00000080
#define BIT_PROGRAM_ERROR	0x00000020
#define BIT_TIMEOUT		0x80000000	/* our flag */

#define READY 1
#define ERR   2
#define TMO   4

/*-----------------------------------------------------------------------
 */
ulong flash_init (void)
{
	int i, j;
	ulong size = 0;

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
		ulong flashbase = 0;

		flash_info[i].flash_id =
#if defined(CONFIG_AMD_LV800)
			(AMD_MANUFACT & FLASH_VENDMASK) |
			(AMD_ID_LV800B & FLASH_TYPEMASK);
#elif defined(CONFIG_MX_LV640EB)
			(MX_MANUFACT & FLASH_VENDMASK) |
			(MX_ID_LV640EB & FLASH_TYPEMASK);
#else
#error "Unknown flash configured"
#endif
		flash_info[i].size = FLASH_BANK_SIZE;
		flash_info[i].sector_count = CFG_MAX_FLASH_SECT;

		memset (flash_info[i].protect, 0, CFG_MAX_FLASH_SECT);
		if (i == 0)
			flashbase = CFG_FLASH_BASE;
		else
			panic ("configured too many flash banks!\n");
		for (j = 0; j < flash_info[i].sector_count; j++) {
#if defined(CONFIG_AMD_LV800)
			if (j <= 3) {
				/* 1st one is 16 KB */
				if (j == 0) {
					flash_info[i].start[j] =
						flashbase + 0;
				}

				/* 2nd and 3rd are both 8 KB */
				if ((j == 1) || (j == 2)) {
					flash_info[i].start[j] =
						flashbase + FIRST_SECT_SIZE + (j - 1) * 
						SECOND_SECT_SIZE;
				}

				/* 4th 32 KB */
				if (j == 3) {
					flash_info[i].start[j] =
						flashbase + FOURTH_SECT_SIZE;
				}
			} else {
				flash_info[i].start[j] =
					flashbase + (j - 3) * MAIN_SECT_SIZE;
			}
#elif defined(CONFIG_MX_LV640EB)
			if (j <=7) {	/* 1st ~ 8th sect size : 8 kb */
				flash_info[i].start[j] = 
					flashbase + j * BOOT_SECT_SIZE;
			} else {	/* 9th ~ 135th sect size : 64 kb */
				flash_info[i].start[j] =
					flashbase + (j - 7) * MAIN_SECT_SIZE;
			}
#else
#error "Unknown flash configured"
#endif
		}
		size += flash_info[i].size;
	}

	flash_protect (FLAG_PROTECT_SET,
		       CFG_FLASH_BASE,
		       CFG_FLASH_BASE + monitor_flash_len - 1,
		       &flash_info[0]);

	flash_protect (FLAG_PROTECT_SET,
		       CFG_ENV_ADDR,
		       CFG_ENV_ADDR + CFG_ENV_SIZE - 1, &flash_info[0]);

	return size;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	int i;

	switch (info->flash_id & FLASH_VENDMASK) {
	case (AMD_MANUFACT & FLASH_VENDMASK):
		printf ("AMD: ");
		break;
	case (MX_MANUFACT & FLASH_VENDMASK):
		printf ("MX: ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case (AMD_ID_LV800B & FLASH_TYPEMASK):
		printf ("1x Amd29LV800BB (8Mbit)\n");
		break;
	case (MX_ID_LV640EB & FLASH_TYPEMASK):
		printf ("1x MX29LV640EB (64Mbit)\n");
		break;		
	default:
		printf ("Unknown Chip Type\n");
		goto Done;
		break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 5) == 0) {
			printf ("\n   ");
		}
		printf (" %08lX%s", info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");

      Done:;
}

/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	ushort result;
	int iflag, cflag, prot, sect;
	int rc = ERR_OK;
	int chip;

	/* first look for protection bits */

	if (info->flash_id == FLASH_UNKNOWN)
		return ERR_UNKNOWN_FLASH_TYPE;

	if ((s_first < 0) || (s_first > s_last)) {
		return ERR_INVAL;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case (AMD_MANUFACT & FLASH_VENDMASK):
		break;
	case (MX_MANUFACT & FLASH_VENDMASK):
		break;
	default:
		return ERR_UNKNOWN_FLASH_VENDOR;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}
	if (prot)
		return ERR_PROTECTED;

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
	//cflag = icache_status ();
	//icache_disable ();
	//iflag = disable_interrupts ();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last && !ctrlc (); sect++) {
		printf ("Erasing sector %2d ... ", sect);

		/* arm simple, non interrupt dependent timer */
		reset_timer_masked ();

		if (info->protect[sect] == 0) {	/* not protected */
			vu_short *addr = (vu_short *) (info->start[sect]);

			MEM_FLASH_RESET = CMD_READ_ARRAY;
			MEM_FLASH_ADDR1 = CMD_UNLOCK1;
			MEM_FLASH_ADDR2 = CMD_UNLOCK2;
			MEM_FLASH_ADDR1 = CMD_ERASE_SETUP;

			MEM_FLASH_ADDR1 = CMD_UNLOCK1;
			MEM_FLASH_ADDR2 = CMD_UNLOCK2;
			*addr = CMD_ERASE_CONFIRM;

			/* wait until flash is ready */
			chip = 0;

			do {
				result = *addr;

				/* check timeout */
				if (get_timer_masked () >
				    CFG_FLASH_ERASE_TOUT) {
					MEM_FLASH_ADDR1 = CMD_READ_ARRAY;
					chip = TMO;
					break;
				}

				if (!chip
				    && (result & 0xFFFF) & BIT_ERASE_DONE)
					chip = READY;

				if (!chip
				    && (result & 0xFFFF) & BIT_PROGRAM_ERROR)
					chip = ERR;

			} while (!chip);

			MEM_FLASH_ADDR1 = CMD_READ_ARRAY;

			if (chip == ERR) {
				rc = ERR_PROG_ERROR;
				goto outahere;
			}
			if (chip == TMO) {
				rc = ERR_TIMOUT;
				goto outahere;
			}

			printf ("ok.\n");
		} else {	/* it was protected */

			printf ("protected!\n");
		}
	}

	if (ctrlc ())
		printf ("User Interrupt!\n");

      outahere:
	/* allow flash to settle - wait 10 ms */
	udelay_masked (10000);

	//if (iflag)
	//	enable_interrupts ();

	//if (cflag)
	//	icache_enable ();

	return rc;
}

/*-----------------------------------------------------------------------
 */

int flash_erase_all (flash_info_t * info)
{
	ushort result;
	int iflag, cflag, prot, sect;
	int rc = ERR_OK;
	int chip;

	/* first look for protection bits */

	if (info->flash_id == FLASH_UNKNOWN)
		return ERR_UNKNOWN_FLASH_TYPE;

	switch (info->flash_id & FLASH_VENDMASK) {
	case (AMD_MANUFACT & FLASH_VENDMASK):
		break;
	case (MX_MANUFACT & FLASH_VENDMASK):
		break;
	default:
		return ERR_UNKNOWN_FLASH_VENDOR;
	}

	printf("flash_erase_all\n");

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
	//cflag = icache_status ();
	//icache_disable ();
	//iflag = disable_interrupts ();

	/* Start erase on unprotected sectors */
	MEM_FLASH_RESET = CMD_READ_ARRAY;
	MEM_FLASH_ADDR1 = CMD_UNLOCK1;
	MEM_FLASH_ADDR2 = CMD_UNLOCK2;
	MEM_FLASH_ADDR1 = CMD_ERASE_SETUP;
	MEM_FLASH_ADDR1 = CMD_UNLOCK1;
	MEM_FLASH_ADDR2 = CMD_UNLOCK2;
	MEM_FLASH_ADDR1 = CMD_ERASE_ALL;

	while(1) {
		if (ctrlc ()) {
			printf ("User Interrupt!\n");
			return rc;
		}

		udelay_masked (10000);
		if (readl(CFG_FLASH_BASE) == 0xffffffff)
			break;
		printf(".");
		}
	printf("\nERASE ALL is DONE\n");

	MEM_FLASH_RESET = CMD_READ_ARRAY;

	//if (iflag)
	//	enable_interrupts ();

	//if (cflag)
	//	icache_enable ();

	return rc;
}


/*-----------------------------------------------------------------------
 * Copy memory to flash
 */

volatile static int write_hword (flash_info_t * info, ulong dest, ushort data)
{
	vu_short *addr = (vu_short *) dest;
	ushort result;
	int rc = ERR_OK;
	int cflag, iflag;
	int chip;

	/*
	 * Check if Flash is (sufficiently) erased
	 */
	result = *addr;
	if ((result & data) != data)
		return ERR_NOT_ERASED;


	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
	cflag = icache_status ();
	icache_disable ();
	iflag = disable_interrupts ();

	MEM_FLASH_ADDR1 = CMD_UNLOCK1;
	MEM_FLASH_ADDR2 = CMD_UNLOCK2;
	MEM_FLASH_ADDR1 = CMD_UNLOCK_BYPASS;
	*addr = CMD_PROGRAM;
	*addr = data;

	/* arm simple, non interrupt dependent timer */
	reset_timer_masked ();

	/* wait until flash is ready */
	chip = 0;
	do {
		result = *addr;

		/* check timeout */
		if (get_timer_masked () > CFG_FLASH_ERASE_TOUT) {
			chip = ERR | TMO;
			break;
		}
		if (!chip && ((result & 0x80) == (data & 0x80)))
			chip = READY;

		if (!chip && ((result & 0xFFFF) & BIT_PROGRAM_ERROR)) {
			result = *addr;

			if ((result & 0x80) == (data & 0x80))
				chip = READY;
			else
				chip = ERR;
		}

	} while (!chip);

	*addr = CMD_READ_ARRAY;

	if (chip == ERR || *addr != data)
		rc = ERR_PROG_ERROR;

	if (iflag)
		enable_interrupts ();

	if (cflag)
		icache_enable ();

	return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 */

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong cp, wp;
	int l;
	int i, rc;
	ushort data;
	ulong checksum;
	uchar *tmp_addr;

	wp = (addr & ~1);	/* get lower word aligned address */

	tmp_addr = src;
#if defined(CONFIG_EVT1)
	/*
	 * in case of S5PV210 EVT1, checksum must be included when writing BL1
	 * to flash
	 *
	 * header info(16bytes) in front of BL1
	 * 0x0 : BL1 size(user write)
	 * 0x4 : Reserved(should be 0)
	 * 0x8 : CheckSum(user write)
	 * 0xc : Reserved(should be 0)
	 */
	if (addr == CFG_FLASH_BASE) {
		tmp_addr += 16;

		for(i = 16, checksum = 0; i < BL1_SIZE; i++) {
			checksum += *(u8*)tmp_addr++;
		}

		*(volatile u32 *)(src + 0x8) = checksum;
	}
#endif

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp) {
			data = (data >> 8) | (*(uchar *) cp << 8);
		}
		for (; i < 2 && cnt > 0; ++i) {
			data = (data >> 8) | (*src++ << 8);
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < 2; ++i, ++cp) {
			data = (data >> 8) | (*(uchar *) cp << 8);
		}

		if ((rc = write_hword (info, wp, data)) != 0) {
			return (rc);
		}
		wp += 2;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 2) {
		data = *((vu_short *) src);
		if ((rc = write_hword (info, wp, data)) != 0) {
			return (rc);
		}
		src += 2;
		wp += 2;
		cnt -= 2;

		if (ctrlc ()) {
			printf ("User Interrupt!\n");
			return rc;
		}

		if ((cnt % 1000) == 0)
			printf(".");
		
	}

	if (cnt == 0) {
		return ERR_OK;
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < 2 && cnt > 0; ++i, ++cp) {
		data = (data >> 8) | (*src++ << 8);
		--cnt;
	}
	for (; i < 2; ++i, ++cp) {
		data = (data >> 8) | (*(uchar *) cp << 8);
	}

	return write_hword (info, wp, data);
}


/*-----------------------------------------------------------------------
 * Copy flash to memory
 */

void read_hword (void)
{
	volatile u32 *buf = CFG_PHY_UBOOT_BASE;
	volatile u32 *nor_base = CFG_FLASH_BASE;
	
	memcpy ((void *)buf, (void *)nor_base, COPY_BL2_SIZE);	
}

