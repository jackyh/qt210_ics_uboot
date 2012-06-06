#include <common.h>
#include <s3c6410.h>

#include <movi.h>
#include <asm/io.h>
#include <regs.h>
#include <mmc.h>

extern raw_area_t raw_area_control;

typedef u32 (*copy_sd_mmc_to_mem) \
	(u32 channel, u32 start_block, u16 block_size, u32* trg, u32 init);

void movi_bl2_copy(void)
{
	copy_sd_mmc_to_mem copy_bl2 = (copy_sd_mmc_to_mem)(*(u32*)(0x0C004008));
	u32 ret;
	
	ret = copy_bl2(0, MOVI_BL2_POS, MOVI_BL2_BLKCNT, CFG_PHY_UBOOT_BASE, 0);

	
	if(ret == 0)
		while(1);
	else
		return;
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
	int i;
	ulong checksum;
	ulong src;
	ulong tmp;

	src = addr;
	
	for(i = 0, checksum = 0;i < SS_SIZE - 4;i++)
	{
		checksum += *(u8*)addr++;
	}

	tmp = *(ulong*)addr;
	*(ulong*)addr = checksum;
			
	movi_write(raw_area_control.image[1].start_blk, 
		raw_area_control.image[1].used_blk, src);

	*(ulong*)addr = tmp;
}

