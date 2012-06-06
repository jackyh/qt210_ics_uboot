/*
 * (C) Copyright 2009-2010 Samsung Electronics Co., Ltd.
 *
 * (C) Copyright 2004
 * Jian Zhang, Texas Instruments, jzhang@ti.com.
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define DEBUG */

#include <common.h>

#if defined(CFG_ENV_IS_IN_AUTO) /* Environment is in Non-Volatile Device */

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <nand.h>
#include <movi.h>
#include <regs.h>

#if defined(CONFIG_CMD_ENV) || \
	defined(CONFIG_CMD_NAND) || \
	defined(CONFIG_CMD_MOVINAND) || \
	defined(CONFIG_CMD_ONENAND)
#define CMD_SAVEENV
#endif

/* info for NAND chips, defined in drivers/nand/nand.c */
extern nand_info_t nand_info[];

/* info for OneNAND chips */
extern struct mtd_info onenand_mtd;

/* info for NOR flash chips, defined in board/samsung/common/flash_common.c */
extern flash_info_t flash_info[];

/* references to names in env_common.c */
extern uchar default_environment[];
extern int default_environment_size;

char * env_name_spec = "SMDK bootable device";

#ifdef ENV_IS_EMBEDDED
extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr = 0;
#endif /* ENV_IS_EMBEDDED */


/* local functions */
#if !defined(ENV_IS_EMBEDDED)
static void use_default(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

uchar env_get_char_spec (int index)
{
	return ( *((uchar *)(gd->env_addr + index)) );
}


/* this is called before nand_init()
 * so we can't read Nand to validate env data.
 * Mark it OK for now. env_relocate() in env_common.c
 * will call our relocate function which will does
 * the real validation.
 *
 * When using a NAND boot image (like sequoia_nand), the environment
 * can be embedded or attached to the U-Boot image in NAND flash. This way
 * the SPL loads not only the U-Boot image from NAND but also the
 * environment.
 */
int env_init(void)
{
#if defined(ENV_IS_EMBEDDED)
	ulong total;
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env1, *tmp_env2;

	total = CFG_ENV_SIZE;

	tmp_env1 = env_ptr;
	tmp_env2 = (env_t *)((ulong)env_ptr + CFG_ENV_SIZE);

	crc1_ok = (crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc);
	crc2_ok = (crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc);

	if (!crc1_ok && !crc2_ok)
		gd->env_valid = 0;
	else if(crc1_ok && !crc2_ok)
		gd->env_valid = 1;
	else if(!crc1_ok && crc2_ok)
		gd->env_valid = 2;
	else {
		/* both ok - check serial */
		if(tmp_env1->flags == 255 && tmp_env2->flags == 0)
			gd->env_valid = 2;
		else if(tmp_env2->flags == 255 && tmp_env1->flags == 0)
			gd->env_valid = 1;
		else if(tmp_env1->flags > tmp_env2->flags)
			gd->env_valid = 1;
		else if(tmp_env2->flags > tmp_env1->flags)
			gd->env_valid = 2;
		else /* flags are equal - almost impossible */
			gd->env_valid = 1;
	}

	if (gd->env_valid == 1)
		env_ptr = tmp_env1;
	else if (gd->env_valid == 2)
		env_ptr = tmp_env2;
#else /* ENV_IS_EMBEDDED */
	gd->env_addr  = (ulong)&default_environment[0];
	gd->env_valid = 1;
#endif /* ENV_IS_EMBEDDED */

	return (0);
}

#ifdef CMD_SAVEENV
/*
 * The legacy NAND code saved the environment in the first NAND device i.e.,
 * nand_dev_desc + 0. This is also the behaviour using the new NAND code.
 */
int saveenv_nand(void)
{
#if defined(CONFIG_CMD_NAND)
        size_t total;
        int ret = 0;
        u32 erasebase;
        u32 eraselength;
        u32 eraseblock;
        u32 erasesize = nand_info[0].erasesize;
        uint8_t *data;

        puts("Erasing Nand...\n");

        /* If the value of CFG_ENV_OFFSET is not a NAND block boundary, the
         * NAND erase operation will fail. So first check if the CFG_ENV_OFFSET
         * is equal to a NAND block boundary
         */
        if ((CFG_ENV_OFFSET % (erasesize - 1)) != 0 ) {
                /* CFG_ENV_OFFSET is not equal to block boundary address. So, read
                 * the read the NAND block (in which ENV has to be stored), and
                 * copy the ENV data into the copied block data.
                 */

                /* Step 1: Find out the starting address of the NAND block to
                 * be erased. Also allocate memory whose size is equal to tbe
                 * NAND block size (NAND erasesize).
                 */
                eraseblock = CFG_ENV_OFFSET / erasesize;
                erasebase = eraseblock * erasesize;
                data = (uint8_t*)malloc(erasesize);
                if (data == NULL) {
                        printf("Could not save enviroment variables\n");
                        return 1;
                }

                /* Step 2: Read the NAND block into which the ENV data has
                 * to be copied
                 */
                total = erasesize;
				ret = nand_read(&nand_info[0], erasebase, &total, data);
                if (ret || total != erasesize) {
                        printf("Could not save enviroment variables %d\n",ret);
                        return 1;
                }

                /* Step 3: Copy the ENV data into the local copy of the block
                 * contents.
                 */
                memcpy((data + (CFG_ENV_OFFSET - erasebase)), (void*)env_ptr, CFG_ENV_SIZE);
        } else {
                /* CFG_ENV_OFFSET is equal to a NAND block boundary. So
                 * no special care is required when erasing and writing NAND
                 * block
                 */
                data = env_ptr;
                erasebase = CFG_ENV_OFFSET;
                erasesize = CFG_ENV_SIZE;
        }

        /* Erase the NAND block which will hold the ENV data */
        if (nand_erase(&nand_info[0], erasebase, erasesize))
                return 1;

        puts("Writing to Nand... \n");
        total = erasesize;

        /* Write the ENV data to the NAND block */
        ret = nand_write(&nand_info[0], erasebase, &total, (u_char*)data);
        if (ret || total != erasesize) {
                printf("Could not save enviroment variables\n");
                return 1;
        }

         if ((CFG_ENV_OFFSET % (erasesize - 1)) != 0 )
                free(data);

        puts("Saved enviroment variables\n");
        return ret;
#else
	return 0;
#endif	/* CONFIG_CMD_NAND */
}

int saveenv_nand_adv(void)
{
#if defined(CONFIG_CMD_NAND)
	size_t total;
	int ret = 0;
	
	u_char *tmp;
	total = CFG_ENV_OFFSET;

	tmp = (u_char *) malloc(total);
	nand_read(&nand_info[0], 0x0, &total, (u_char *) tmp);

	puts("Erasing Nand...");
	nand_erase(&nand_info[0], 0x0, CFG_ENV_OFFSET + CFG_ENV_SIZE);

//#ifndef CONFIG_S5PC100_EVT1
	if (nand_erase(&nand_info[0], 0x0, CFG_ENV_OFFSET + CFG_ENV_SIZE)) {
		free(tmp);
		return 1;
	}
//#endif

	puts("Writing to Nand... ");
	ret = nand_write(&nand_info[0], 0x0, &total, (u_char *) tmp);
	total = CFG_ENV_SIZE;

	ret = nand_write(&nand_info[0], CFG_ENV_OFFSET, &total, (u_char *) env_ptr);

//#ifndef CONFIG_S5PC100_EVT1
	if (ret || total != CFG_ENV_SIZE) {
		free(tmp);
		return 1;
	}
//#endif
	puts("done\n");
	free(tmp);

	return ret;
#else
	return 0;
#endif	/* CONFIG_CMD_NAND */
}

int saveenv_movinand(void)
{
#if defined(CONFIG_CMD_MOVINAND)
        movi_write_env(virt_to_phys((ulong)env_ptr));
        puts("done\n");

        return 1;
#else
	return 0;
#endif	/* CONFIG_CMD_MOVINAND */
}

int saveenv_onenand(void)
{
#if defined(CONFIG_MTD_ONENAND)
	size_t total;
	int ret = 1;
	struct mtd_info *onenand = &onenand_mtd;
	struct erase_info instr;
	u32 erasebase;
	u32 erasesize = onenand_mtd.erasesize;
	u32 writesize = erasesize;
	u_char *data = NULL;
	total = CFG_ENV_OFFSET;

	/* If the value of CFG_ENV_OFFSET is not a OneNAND block boundary, the
	 * OneNAND erase operation will fail. So first check if the CFG_ENV_OFFSET
	 * is equal to a NAND block boundary
	 */
	if ((CFG_ENV_OFFSET % (erasesize - 1)) != 0 ) {
		/* CFG_ENV_OFFSET is not equal to block boundary address.
		 * So, read the OneNAND block (in which ENV has to be stored),
		 * and copy the ENV data into the copied block data.
		 */

		/* Step 1: Find out the starting address of the OneNAND block to
		 * be erased. Also allocate memory whose size is equal to tbe
		 * OneNAND block size (OneNAND erasesize).
		 */
		erasebase = (CFG_ENV_OFFSET / erasesize) * erasesize;
		data = (uint8_t*)malloc(erasesize);
		if (data == NULL) {
			printf("Could not save enviroment variables\n");
			return 1;
		}

		/* Step 2: Read the OneNAND block into which the ENV data has
		 * to be copied
		 */
		ret = onenand->read(onenand, erasebase, erasesize, &total, (u_char *) data);
		if (ret || total != erasesize) {
			printf("Could not save enviroment variables %d\n",ret);
			goto err;
		}

		/* Step 3: Copy the ENV data into the local copy of the block
		 * contents.
		 */
		memcpy((data + (CFG_ENV_OFFSET - erasebase)), (void*) env_ptr, CFG_ENV_SIZE);
	} else {
		/* CFG_ENV_OFFSET is equal to a OneNAND block boundary. So
		 * no special care is required when erasing and writing OneNAND
		 * block
		 */
		data = env_ptr;
		erasebase = CFG_ENV_OFFSET;
		writesize = CFG_ENV_SIZE;
	}

	/* Erase the OneNAND block which will hold the ENV data */
	instr.mtd = onenand;
	instr.addr = erasebase;
	instr.len = erasesize;
	instr.callback = 0;
	puts("Erasing OneNAND...\n");
	if (onenand->erase(onenand, &instr)) {
		printf("Could not save enviroment variables\n");
		goto err;
	}

	/* Write the ENV data to the OneNAND block */
	puts("Writing to OneNAND...\n");
	ret = onenand->write(onenand, erasebase, writesize, &total, (u_char *) data);
	if (ret || total != erasesize) {
		printf("Could not save enviroment variables\n");
		goto err;
	}

	puts("Saved enviroment variables\n");
	ret = 0;
err:
	free(data);
	return ret;
#else
	printf("OneNAND does not support the saveenv command\n");
	return 1;
#endif	/* CONFIG_MTD_ONENAND */
}

int saveenv_nor(void)
{
	u32 sect_num;
	u32 sect_size;
	u32 pos_env;
	u8 *tmp_buf, *tmp_pos_env;
	ulong from, to;	/* unprotect area */
	flash_info_t *info;
	
	info = &flash_info[0];	/* NOR flash is located in #1 bank */

	/* find sector for saving environment variables */
	pos_env = CFG_FLASH_BASE + CFG_ENV_ADDR + CFG_ENV_OFFSET;
	for (sect_num = 0; sect_num < info->sector_count; sect_num++) {
		if ((info->start[sect_num] <= pos_env) &&
			(info->start[sect_num + 1] > pos_env)) {
			break;
		}
	}

	/* unprotect finding sector */
	from = info->start[sect_num];
	to = info->start[sect_num + 1] - 1;
	flash_protect(0, from, to, info); /* unprotect */

	/*
	 * read 64kb one sector from NOR flash to memory
	 * because env var is inserted into unprotected sector
	 */
	sect_size = info->start[sect_num + 1] - info->start[sect_num];
	tmp_buf = (u8 *)malloc(sect_size);
	memcpy(tmp_buf, (u8 *)info->start[sect_num], sect_size);
	tmp_pos_env = tmp_buf + pos_env - info->start[sect_num];
	memcpy(tmp_pos_env, (u8 *)env_ptr, CFG_ENV_SIZE);
	
	/* erase unprotected sector */
	flash_erase(info, sect_num, sect_num);

	/* write modified sector including environment variables */
	flash_write(tmp_buf, (ulong)info->start[sect_num], sect_size);

	free(tmp_buf);
        printf("done\n");
	
        return 1;
}
int saveenv(void)
{
#if defined(CONFIG_S5PC100) || defined(CONFIG_S5PC110) || defined(CONFIG_S5P6442)
	if (INF_REG3_REG == 2)
		saveenv_nand();
	else if (INF_REG3_REG == 3)
		saveenv_movinand();
	else if (INF_REG3_REG == 1)
		saveenv_onenand();
	else if (INF_REG3_REG == 4)
		saveenv_nor();
#elif	defined(CONFIG_SMDK6440)
	if (INF_REG3_REG == 3)
		saveenv_nand();
	else if (INF_REG3_REG == 4 || INF_REG3_REG == 5 || INF_REG3_REG == 6)
		saveenv_nand_adv();
	else if (INF_REG3_REG == 0 || INF_REG3_REG == 1 || INF_REG3_REG == 7)
		saveenv_movinand();
#else   // others
	if (INF_REG3_REG == 2 || INF_REG3_REG == 3)
		saveenv_nand();
	else if (INF_REG3_REG == 4 || INF_REG3_REG == 5 || INF_REG3_REG == 6)
		saveenv_nand_adv();
	else if (INF_REG3_REG == 0 || INF_REG3_REG == 7)
		saveenv_movinand();
	else if (INF_REG3_REG == 1)
		saveenv_onenand();
#endif
	else
		printf("Unknown boot device\n");

	return 0;
}
#endif /* CMD_SAVEENV */

/*
 * The legacy NAND code saved the environment in the first NAND device i.e.,
 * nand_dev_desc + 0. This is also the behaviour using the new NAND code.
 */
void env_relocate_spec_nand(void)
{
#if !defined(ENV_IS_EMBEDDED)
#if defined(CONFIG_CMD_NAND)
	size_t total;
	int ret;
	u_char *data;

	data = (u_char*)malloc(CFG_ENV_SIZE);
	total = CFG_ENV_SIZE;
	ret = nand_read(&nand_info[0], CFG_ENV_OFFSET, &total, (u_char*)data);
	env_ptr = data;

	if (ret || total != CFG_ENV_SIZE)
		return use_default();
	if (crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc)
		return use_default();
#else
	use_default();
#endif /* CONFIG_CMD_NAND */
#endif /* ! ENV_IS_EMBEDDED */
}

void env_relocate_spec_nor(void)
{
#if !defined(ENV_IS_EMBEDDED)
	size_t total;
	int ret, i;
	u_char *env_dst, *env_src;

	env_src = (u_char *)(CFG_FLASH_BASE + CFG_ENV_OFFSET);
	env_dst = (u_char *)malloc(CFG_ENV_SIZE);
	total = CFG_ENV_SIZE;
	
	memcpy(env_dst, env_src, total);
	env_ptr = env_dst;
	
	if (crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc)
		return use_default();
#endif
}

void env_relocate_spec_movinand(void)
{
#if !defined(ENV_IS_EMBEDDED)
#if defined(CONFIG_CMD_MOVINAND)
	uint *magic = (uint*)(PHYS_SDRAM_1);

	if ((0x24564236 != magic[0]) || (0x20764316 != magic[1])) {
		movi_read_env(virt_to_phys((ulong)env_ptr));
	}
	
	if (crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc)
		return use_default();
#endif /* CONFIG_CMD_MOVINAND */
#endif /* ! ENV_IS_EMBEDDED */
}

void env_relocate_spec_onenand(void)
{
#if !defined(ENV_IS_EMBEDDED)
#if defined(CONFIG_MTD_ONENAND)
	size_t total;
	int ret;
	struct mtd_info *onenand = &onenand_mtd;

	ret = onenand->read(onenand, CFG_ENV_OFFSET, CFG_ENV_SIZE, &total, (u_char*)env_ptr);
	if (ret || total != CFG_ENV_SIZE)
		return use_default();

	if (crc32(0, env_ptr->data, ENV_SIZE) != env_ptr->crc)
		return use_default();
#else
	use_default();
#endif /* CONFIG_MTD_ONENAND */
#endif /* ! ENV_IS_EMBEDDED */
}

void env_relocate_spec(void)
{
#if defined(CONFIG_SMDKC100) | defined(CONFIG_SMDKC110) | defined(CONFIG_S5P6442)
	if (INF_REG3_REG == 1)
		env_relocate_spec_onenand();
	else if (INF_REG3_REG == 2)
		env_relocate_spec_nand();
	else if (INF_REG3_REG == 3)
		env_relocate_spec_movinand();
	else if (INF_REG3_REG == 4)
		env_relocate_spec_nor();
	else
		use_default();
#elif !defined(CONFIG_SMDK6440)
	if (INF_REG3_REG >= 2 && INF_REG3_REG <= 6)
		env_relocate_spec_nand();
	else if (INF_REG3_REG == 0 || INF_REG3_REG == 7)
		env_relocate_spec_movinand();
	else if (INF_REG3_REG == 1)
		env_relocate_spec_onenand();
	else
		printf("Unknown boot device\n");
#else
	if (INF_REG3_REG >= 3 && INF_REG3_REG <= 6)
		env_relocate_spec_nand();
	else if (INF_REG3_REG == 0 || INF_REG3_REG == 1 || INF_REG3_REG == 7)
		env_relocate_spec_movinand();
#endif
}

#if !defined(ENV_IS_EMBEDDED)
static void use_default()
{
	puts("*** Warning - using default environment\n\n");

	if (default_environment_size > CFG_ENV_SIZE) {
		puts("*** Error - default environment is too large\n\n");
		return;
	}

	memset (env_ptr, 0, sizeof(env_t));
	memcpy (env_ptr->data,
			default_environment,
			default_environment_size);
	env_ptr->crc = crc32(0, env_ptr->data, ENV_SIZE);
	gd->env_valid = 1;

}
#endif

#endif /* CFG_ENV_IS_IN_NAND */
