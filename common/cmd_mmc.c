/*
 * (C) Copyright 2003
 * Kyle Harris, kharris@nexus-tech.net
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
#include <command.h>
#include <mmc.h>

#define MAXIMUM_BLOCK_COUNT 0xFFFF

#ifndef CONFIG_GENERIC_MMC
int do_mmc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (mmc_init (1) != 0) {
		printf ("No MMC card found\n");
		return 1;
	}
	return 0;
}

U_BOOT_CMD(
	mmcinit,	1,	0,	do_mmc,
	"mmcinit - init mmc card\n",
	NULL
);
#else /* !CONFIG_GENERIC_MMC */

static void print_mmcinfo(struct mmc *host)
{
	printf("Device: %s\n", host->name);
	printf("Manufacturer ID: %x\n", host->cid[0] >> 24);
	printf("RCA: %x\n", host->rca);
	printf("OEM: %x\n", (host->cid[0] >> 8) & 0xffff);
	printf("Name: %c%c%c%c%c \n", host->cid[0] & 0xff,
			(host->cid[1] >> 24), (host->cid[1] >> 16) & 0xff,
			(host->cid[1] >> 8) & 0xff, host->cid[1] & 0xff);

	printf("Tran Speed: %d\n", host->clock);
	printf("Rd Block Len: %d\n", host->read_bl_len);

	printf("%s version %d.%d\n", IS_SD(host) ? "SD" : "MMC",
			(host->version >> 4) & 0xf, host->version & 0xf);

	printf("High Capacity: %s\n", host->high_capacity ? "Yes" : "No");
	printf("Size: %dMB (block: %d)\n",
			(host->capacity/(1024*1024/(1<<9))),
			host->capacity);

	printf("Bus Width: %d-bit\n\n", host->bus_width);
}

int do_mmcinfo (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	struct mmc *mmc;
	int dev_num;
	int err;

	if (argc < 2)
		dev_num = 0;
	else
		dev_num = simple_strtoul(argv[1], NULL, 0);

	mmc = find_mmc_device(dev_num);

	if (mmc) {
		err = mmc_init(mmc);
		if(err)
			return err;

		print_mmcinfo(mmc);
	}

	return 0;
}

U_BOOT_CMD(mmcinfo,	2,	0,	do_mmcinfo,
	"print MMC information",
	"<dev num>\n");

int do_mmcops(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rc = 0;

	switch (argc) {
	case 3:
		if (strcmp(argv[1], "rescan") == 0) {
			int dev = simple_strtoul(argv[2], NULL, 10);
			struct mmc *mmc = find_mmc_device(dev);

			if (!mmc)
				return 1;

			rc = mmc_init(mmc);
			return rc;
		}

	case 0:
	case 1:
	case 4:
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;

	case 2:
		if (!strcmp(argv[1], "list")) {
			print_mmc_devices('\n');
			return 0;
		}
		return 1;
	default: /* at least 5 args */
		if (strcmp(argv[1], "read") == 0) {
			int dev = simple_strtoul(argv[2], NULL, 10);
			void *addr = (void *)simple_strtoul(argv[3], NULL, 16);
			u32 cnt = simple_strtoul(argv[5], NULL, 10);
			u32 blk = simple_strtoul(argv[4], NULL, 10);
			u32 n;
			u32 read_cnt;
			u32 cnt_to_read;
			void *addr_to_read;
			struct mmc *mmc = find_mmc_device(dev);

			if (!mmc)
				return 1;

			printf("\nMMC read: dev # %d, block # %d, count %d ...",
				dev, blk, cnt);

			rc = mmc_init(mmc);
			if(rc)
				return rc;

			n = 0;
			addr_to_read = addr;
			do {
				if (cnt - n > MAXIMUM_BLOCK_COUNT)
					cnt_to_read = MAXIMUM_BLOCK_COUNT;
				else
					cnt_to_read = cnt - n;

				read_cnt = mmc->block_dev.block_read(dev, blk, cnt_to_read, addr_to_read);
				n += read_cnt;
				blk += read_cnt;
				addr_to_read += read_cnt * 512;
				if(cnt_to_read != read_cnt) {
					printf("%d blocks read: %s\n",
						n, "ERROR");
					return -1;
				}
			} while(cnt > n);

			/* flush cache after read */
			flush_cache((ulong)addr, cnt * 512); /* FIXME */

			printf("%d blocks read: %s\n",
				n, "OK");
			return 0;
		} else if (strcmp(argv[1], "write") == 0) {
			int dev = simple_strtoul(argv[2], NULL, 10);
			void *addr = (void *)simple_strtoul(argv[3], NULL, 16);
			u32 cnt = simple_strtoul(argv[5], NULL, 10);
			int blk = simple_strtoul(argv[4], NULL, 10);
			u32 n;
			u32 written_cnt;
			u32 cnt_to_write;
			void *addr_to_write = addr;
			struct mmc *mmc = find_mmc_device(dev);


			if (!mmc)
				return 1;

			printf("\nMMC write: dev # %d, block # %d, count %d ... ",
				dev, blk, cnt);

			rc = mmc_init(mmc);
			if(rc)
				return rc;

			n = 0;
			addr_to_write = addr;
			do {
				if (cnt - n > MAXIMUM_BLOCK_COUNT)
					cnt_to_write = MAXIMUM_BLOCK_COUNT;
				else
					cnt_to_write = cnt - n;

				written_cnt = mmc->block_dev.block_write(dev, blk, cnt_to_write, addr_to_write);
				n += written_cnt;
				blk += written_cnt;
				addr_to_write += written_cnt * 512;
				if(cnt_to_write != written_cnt) {
					printf("%d blocks written: %s\n",
						n, "ERROR");
					return -1;
				}
			} while(cnt > n);

			printf("%d blocks written: %s\n",
				n, "OK");
			return 0;
		} else {
			printf("Usage:\n%s\n", cmdtp->usage);
			rc = 1;
		}

		return rc;
	}
}

U_BOOT_CMD(
	mmc, 6, 1, do_mmcops,
	"MMC sub system",
	"read <device num> addr blk# cnt\n"
	"mmc write <device num> addr blk# cnt\n"
	"mmc rescan <device num>\n"
	"mmc list - list available devices\n");
#endif
