#include <common.h>
#include <s5pc100.h>

#include <movi.h>
#include <asm/io.h>
#include <regs.h>

#ifndef CONFIG_GENERIC_MMC

uint movi_hc = 0;

void movi_set_capacity(void)
{
	if (MOVI_HIGH_CAPACITY & 0x1)
		movi_hc = 1;
}

int movi_set_ofs(uint last)
{
	int changed = 0;

	if (ofsinfo.last != last) {
		ofsinfo.last 	= last - (eFUSE_SIZE / MOVI_BLKSIZE);
		ofsinfo.bl1	= ofsinfo.last - MOVI_BL1_BLKCNT;
		ofsinfo.env	= ofsinfo.bl1 - MOVI_ENV_BLKCNT;
		ofsinfo.bl2	= ofsinfo.bl1 - (MOVI_BL2_BLKCNT + MOVI_ENV_BLKCNT);
		ofsinfo.kernel	= ofsinfo.bl2 - MOVI_ZIMAGE_BLKCNT;
		ofsinfo.rootfs	= ofsinfo.kernel - MOVI_ROOTFS_BLKCNT;
		changed = 1;
	}

	return changed;
}

int movi_init(void)
{
	int ret = 0;

        hsmmc_set_gpio();
        hsmmc_set_base();
        hsmmc_set_clock();
        hsmmc_reset();
        ret = hsmmc_init();
	if (ret) {
		printf("\nCard Initialization failed(%d)\n", ret);
		return -1;
	}

	return 1;
}

void movi_write_env(ulong addr)
{
	movi_write((uint)addr, ofsinfo.env, MOVI_ENV_BLKCNT);
}

void movi_read_env(ulong addr)
{
	movi_read(addr, ofsinfo.env, MOVI_ENV_BLKCNT);
}

typedef u32 (*MMC_ReadBlocks)(u32, u32, u32);
void movi_bl2_copy(void)
{
	MMC_ReadBlocks readmmc = (MMC_ReadBlocks)0x1F8;	
	readmmc(BL2_BASE, MOVI_BL2_POS, MOVI_BL2_BLKCNT);
}

#else	/* !CONFIG_GENERIC_MMC */

#include <mmc.h>

extern raw_area_t raw_area_control;

typedef u32 (*MMC_ReadBlocks)(u32, u32, u32);
void movi_bl2_copy(void)
{
	MMC_ReadBlocks readmmc = (MMC_ReadBlocks)0x1F8;

	readmmc(CFG_PHY_UBOOT_BASE, MOVI_BL2_POS, MOVI_BL2_BLKCNT);

	/* To clear card interrupt in SD/MMC controller */
	writew(0x100, ELFIN_HSMMC_BASE+HM_NORINTSTS);
}

void print_movi_bl2_info (void)
{
	printf("%d, %d, %d\n", MOVI_BL2_POS, MOVI_BL2_BLKCNT, MOVI_ENV_BLKCNT);
}

void movi_write_env(ulong addr)
{
	movi_write(raw_area_control.image[1].start_blk, 
		raw_area_control.image[1].used_blk, addr);
}

void movi_read_env(ulong addr)
{
	movi_read(raw_area_control.image[1].start_blk,
		raw_area_control.image[1].used_blk, addr);
}

void movi_write_bl1(ulong addr)
{
	movi_write(raw_area_control.image[1].start_blk, 
		raw_area_control.image[1].used_blk, addr);
}

#endif /* !CONFIG_GENERIC_MMC */

