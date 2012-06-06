#ifndef __MOVI_H__
#define __MOVI_H__

#ifndef CONFIG_GENERIC_MMC

#define MOVI_INIT_REQUIRED	0

#if defined(CONFIG_S3C6400) || defined(CONFIG_S3C6410) || defined(CONFIG_S3C6430)
#define	TCM_BASE		0x0C004000
#define BL2_BASE		0x57E00000
#elif defined(CONFIG_S3C2450) || defined(CONFIG_S3C2416)
#define	TCM_BASE		0x40004000
#define BL2_BASE		0x33E00000
#elif defined(CONFIG_S5P6440)
#define S5P6440_SDMMC_BASE	0xd0021c4c
#define S5P6440_SDHC_INFO	0xd0021c50
#define S5P6440_BLK_SIZE	0xd0021c54
#define BL2_BASE		0x27e00000
#define S5P6440_DEVCPY_BASE	0xd0021c00
#elif defined(CONFIG_S5PC100) || defined(CONFIG_S5PC110) || defined(CONFIG_S5P6442)
#define S5PC100_BLK_SIZE        0xd0020230
#define S5PC100_SDHC_INFO       0xd002011F
#define BL2_BASE                0x27e00000
#else
# error TCM_BASE or BL2_BASE is not defined
#endif

/* TCM function for bl2 load */
#if defined(CONFIG_S3C6400)
#define CopyMovitoMem(a,b,c,d,e)	(((int(*)(uint, ushort, uint *, uint, int))(*((uint *)(TCM_BASE + 0x8))))(a,b,c,d,e))
#elif defined(CONFIG_S3C6410) || defined(CONFIG_S3C6430)
#define CopyMovitoMem(a,b,c,d,e)	(((int(*)(int, uint, ushort, uint *, int))(*((uint *)(TCM_BASE + 0x8))))(a,b,c,d,e))
#elif defined(CONFIG_S5P6440)
#define COPY_eMMC_to_MEM(a,b,c,d)	(((void(*)(uint, uint, uint *, int))(*((uint *)(S5P6440_DEVCPY_BASE + 0xc))))(a,b,c,d))
#define CopyMovitoMem(a,b,c,d,e)	(((int(*)(int, uint, ushort, uint *, int))(*((uint *)(S5P6440_DEVCPY_BASE + 0x8))))(a,b,c,d,e))
#elif defined(CONFIG_S3C2450) || defined(CONFIG_S3C2416)
#define CopyMovitoMem(a,b,c,d)		(((int(*)(uint, ushort, uint *, int))(*((uint *)(TCM_BASE + 0x8))))(a,b,c,d))
#endif

/* size information */
#if defined(CONFIG_S3C6400)
#define SS_SIZE			(4 * 1024)
#define eFUSE_SIZE		(2 * 1024)	// 1.5k eFuse, 0.5k reserved
#else
#define SS_SIZE			(8 * 1024)
#define eFUSE_SIZE		(1 * 1024)	// 0.5k eFuse, 0.5k reserved
#endif

/* movinand definitions */
#define MOVI_BLKSIZE		512
#define MOVI_UBOOTSIZE		496
#define MOVI_RW_MAXBLKS		40960
#if defined(CONFIG_S5P6440)
#define MOVI_TOTAL_BLKCNT	*((volatile unsigned int*)(S5P6440_BLK_SIZE))
#define MOVI_HIGH_CAPACITY	*((volatile unsigned int*)(S5P6440_SDHC_INFO))
#elif defined(CONFIG_S5PC100) || defined(CONFIG_S5PC110) || defined(CONFIG_S5P6442)
#define MOVI_TOTAL_BLKCNT       *((volatile unsigned int*)(S5PC100_BLK_SIZE))
#define MOVI_HIGH_CAPACITY      *((volatile unsigned char*)(S5PC100_SDHC_INFO))
#else
#define MOVI_TOTAL_BLKCNT	*((volatile unsigned int*)(TCM_BASE - 0x4))
#define MOVI_HIGH_CAPACITY	*((volatile unsigned int*)(TCM_BASE - 0x8))
#endif

/* partition information */
#define PART_SIZE_BL		(512 * 1024)
#define PART_SIZE_UBOOT		(496 * 1024)
#define PART_SIZE_KERNEL	(4 * 1024 * 1024)
#define PART_SIZE_ROOTFS	(8 * 1024 * 1024)

#define MOVI_LAST_BLKPOS	(MOVI_TOTAL_BLKCNT - (eFUSE_SIZE / MOVI_BLKSIZE))
#define MOVI_BL1_BLKCNT		(SS_SIZE / MOVI_BLKSIZE)
#define MOVI_ENV_BLKCNT		(CFG_ENV_SIZE / MOVI_BLKSIZE)
#define MOVI_BL2_BLKCNT		(PART_SIZE_BL / MOVI_BLKSIZE)
#define MOVI_ZIMAGE_BLKCNT	(PART_SIZE_KERNEL / MOVI_BLKSIZE)
#define MOVI_BL2_POS		(MOVI_LAST_BLKPOS - MOVI_BL1_BLKCNT - MOVI_BL2_BLKCNT - MOVI_ENV_BLKCNT)
#define MOVI_ZIMAGE_POS		(MOVI_BL2_POS - MOVI_ZIMAGE_BLKCNT)
#define MOVI_ROOTFS_BLKCNT	(PART_SIZE_ROOTFS / MOVI_BLKSIZE)

/* eMMC partition information */
#define EMMC_BL_SIZE		(512 * 1024)	/* UBOOT + ENV */
#define EMMC_UBOOT_SIZE		(496 * 1024)
#define EMMC_ENV_SIZE		(16 * 1024)
#define EMMC_KERNEL_SIZE	((3 * 1024 * 1024) + (512 * 1024))
#define EMMC_ROOTFS_SIZE	(12 * 1024 * 1024)

#define EMMC_BL_BLKCNT		(EMMC_BL_SIZE / MOVI_BLKSIZE)
#define EMMC_UBOOT_BLKCNT	(EMMC_UBOOT_SIZE / MOVI_BLKSIZE)
#define EMMC_ENV_BLKCNT		(EMMC_ENV_SIZE / MOVI_BLKSIZE)
#define EMMC_KERNEL_BLKCNT	(EMMC_KERNEL_SIZE / MOVI_BLKSIZE)
#define EMMC_ROOTFS_BLKCNT	(EMMC_ROOTFS_SIZE / MOVI_BLKSIZE)

struct movi_offset_t {
	uint	last;
	uint	bl1;
	uint	env;
	uint	bl2;
	uint	kernel;
	uint	rootfs;
};

/* external functions */
extern void hsmmc_set_base(void);
extern void hsmmc_set_gpio(void);
extern void hsmmc_reset (void);
extern int hsmmc_init (void);
extern int movi_init(void);
extern void movi_set_capacity(void);
extern int movi_set_ofs(uint last);
extern void movi_write (uint addr, uint start_blk, uint blknum);
extern void movi_read (uint addr, uint start_blk, uint blknum);
extern void movi_write_env(ulong addr);
extern void movi_read_env(ulong addr);

extern int emmc_write(uint addr, uint start_blk, uint blk_num);
extern void emmc_read(uint addr, uint start_blk, uint blk_num);

#if defined(CONFIG_S3C2450)
extern ulong virt_to_phy_smdk2450(ulong addr);
#elif defined(CONFIG_S3C6400)
extern ulong virt_to_phy_smdk6400(ulong addr);
#elif defined(CONFIG_S3C6410)
extern ulong virt_to_phy_smdk6410(ulong addr);
#elif defined(CONFIG_S3C6430)
extern ulong virt_to_phy_smdk6430(ulong addr);
#elif defined(CONFIG_S3C2416)
extern ulong virt_to_phy_smdk2416(ulong addr);
#elif defined(CONFIG_S5P6440)
extern ulong virt_to_phy_smdk6440(ulong addr);
#endif

extern void test_hsmmc (uint width, uint test, uint start_blk, uint blknum);

/* external variables */
extern int movi_ch;
extern int movi_emmc;
extern uint movi_hc;
extern struct movi_offset_t ofsinfo;

#else	/* !CONFIG_GENERIC_MMC */

#define MAGIC_NUMBER_MOVI	(0x24564236)

#define MOVI_TOTAL_BLKCNT       *((volatile unsigned int*)(SDMMC_BLK_SIZE))

#if defined(CONFIG_SECURE) || defined(CONFIG_FUSED)
#define FWBL1_SIZE		(4* 1024)
#endif

#define SS_SIZE			(8 * 1024)

#if defined(CONFIG_EVT1)
#define eFUSE_SIZE		(1 * 512)	// 512 Byte eFuse, 512 Byte reserved
#else
#define eFUSE_SIZE		(1 * 1024)	// 1 kB eFuse, 1 KB reserved
#endif /* CONFIG_EVT1 */


#define MOVI_BLKSIZE		(1<<9) /* 512 bytes */

/* partition information */
#define PART_SIZE_BL		(512 * 1024)
#define PART_SIZE_KERNEL	(4 * 1024 * 1024)
#define PART_SIZE_ROOTFS	(26 * 1024 * 1024)

#define MOVI_LAST_BLKPOS	(MOVI_TOTAL_BLKCNT - (eFUSE_SIZE / MOVI_BLKSIZE))

/* Add block count at fused chip */
#if defined(CONFIG_SECURE) || defined(CONFIG_FUSED)
#define MOVI_FWBL1_BLKCNT	(FWBL1_SIZE / MOVI_BLKSIZE)	/* 4KB */
#endif

#define MOVI_BL1_BLKCNT		(SS_SIZE / MOVI_BLKSIZE)	/* 8KB */
#define MOVI_ENV_BLKCNT		(CFG_ENV_SIZE / MOVI_BLKSIZE)	/* 16KB */
#define MOVI_BL2_BLKCNT		(PART_SIZE_BL / MOVI_BLKSIZE)	/* 512KB */
#define MOVI_ZIMAGE_BLKCNT	(PART_SIZE_KERNEL / MOVI_BLKSIZE)	/* 4MB */

/* Change writing block position at fused chip */
#if defined(CONFIG_EVT1)
	#if defined(CONFIG_SECURE) || defined(CONFIG_FUSED)
#define MOVI_BL2_POS		((eFUSE_SIZE / MOVI_BLKSIZE) + (FWBL1_SIZE / MOVI_BLKSIZE) + MOVI_BL1_BLKCNT + MOVI_ENV_BLKCNT)
	#else
#define MOVI_BL2_POS		((eFUSE_SIZE / MOVI_BLKSIZE) + MOVI_BL1_BLKCNT + MOVI_ENV_BLKCNT)
	#endif
#else
#define MOVI_BL2_POS		(MOVI_LAST_BLKPOS - MOVI_BL1_BLKCNT - MOVI_BL2_BLKCNT - MOVI_ENV_BLKCNT)
#endif

// #define MOVI_ZIMAGE_POS		(MOVI_BL2_POS - MOVI_ZIMAGE_BLKCNT)
#define MOVI_ROOTFS_BLKCNT	(PART_SIZE_ROOTFS / MOVI_BLKSIZE)

/*
 *
 * start_blk: start block number for image
 * used_blk: blocks occupied by image
 * size: image size in bytes
 * attribute: attributes of image
 *            0x1: u-boot parted (BL1)
 *            0x2: u-boot (BL2)
 *            0x4: kernel
 *            0x8: root file system
 *            0x10: environment area
 *            0x20: reserved
 * description: description for image
 * by scsuh
 */
typedef struct member {
	uint start_blk;
	uint used_blk;
	uint size;
	uint attribute; /* attribute of image */
	char description[16];
} member_t; /* 32 bytes */

/*
 * magic_number: 0x24564236
 * start_blk: start block number for raw area
 * total_blk: total block number of card
 * next_raw_area: add next raw_area structure
 * description: description for raw_area
 * image: several image that is controlled by raw_area structure
 * by scsuh
 */
typedef struct raw_area {
	uint magic_number; /* to identify itself */
	uint start_blk; /* compare with PT on coherency test */
	uint total_blk;
	uint next_raw_area; /* should be sector number */
	char description[16];
	member_t image[15];
} raw_area_t; /* 512 bytes */

#endif /* !CONFIG_GENERIC_MMC */

#endif /*__MOVI_H__*/
