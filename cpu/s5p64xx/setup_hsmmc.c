#include <common.h>
#include <regs.h>
#include <asm/io.h>
#include <mmc.h>
#include <s3c_hsmmc.h>

void setup_hsmmc_clock(void)
{
	u32 tmp;
	u32 clock;
	u32 i;
#ifdef OM_PIN
	if(OM_PIN == SDMMC_CHANNEL0) {
		/* MMC0 clock src = SCLKMPLL */
		tmp = CLK_SRC0_REG & ~(0x3<<18);
		CLK_SRC0_REG = tmp | (0x1<<18);

		/* MMC0 clock div */
		tmp = CLK_DIV1_REG & ~(0xf<<0);
		clock = get_MPLL_CLK()/1000000;

		for(i=0; i<0xf; i++)
		{
			if((clock / (i+1)) <= 50) {
				CLK_DIV1_REG = tmp | i<<0;
				break;
			}
		}
		return ;
	} else if (OM_PIN == SDMMC_CHANNEL1) {
		/* MMC1 clock src = SCLKMPLL */
		tmp = CLK_SRC0_REG & ~(0x3<<20);
		CLK_SRC0_REG = tmp | (0x1<<20);


		/* MMC1 clock div */
		tmp = CLK_DIV1_REG & ~(0xf<<4);
		clock = get_MPLL_CLK()/1000000;
		for(i=0; i<0xf; i++)
		{
			if((clock / (i+1)) <= 50) {
				CLK_DIV1_REG = tmp | i<<4;
				break;
			}
		}
		return ;
	}
#endif

#ifdef USE_MMC0
	/* MMC0 clock src = SCLKMPLL */
	tmp = CLK_SRC0_REG & ~(0x3<<18);
	CLK_SRC0_REG = tmp | (0x1<<18);


	/* MMC0 clock div */
	tmp = CLK_DIV1_REG & ~(0xf);
	clock = get_MPLL_CLK()/1000000;
	for(i=0; i<0xf; i++)
	{
		if((clock / (i+1)) <= 50) {
			CLK_DIV1_REG = tmp | i<<0;
			break;
		}
	}
#endif

#ifdef USE_MMC1
	/* MMC1 clock src = SCLKMPLL */
	tmp = CLK_SRC0_REG & ~(0x3<<20);
	CLK_SRC0_REG = tmp | (0x1<<20);

	/* MMC1 clock div */
	tmp = CLK_DIV1_REG & ~(0xf<<4);
	clock = get_MPLL_CLK()/1000000;
	for(i=0; i<0xf; i++)
	{
		if((clock / (i+1)) <= 50) {
			CLK_DIV1_REG = tmp | i<<4;
			break;
		}
	}
#endif

#ifdef USE_MMC2
	/* MMC2 clock src = SCLKMPLL */
	tmp = CLK_SRC0_REG & ~(0x3<<22);
	CLK_SRC0_REG = tmp | (0x1<<22);

	/* MMC2 clock div */
	tmp = CLK_DIV1_REG & ~(0xf<<8);
	clock = get_MPLL_CLK()/1000000;
	for(i=0; i<0xf; i++)
	{
		if((clock / (i+1)) <= 50) {
			CLK_DIV1_REG = tmp | i<<8;
			break;
		}
	}
#endif
}

/*
 * this will set the GPIO for hsmmc ch0
 * GPG0[0:6] = CLK, CMD, DAT[0:3], CDn
 */
void setup_hsmmc0_cfg_gpio(void)
{
	ulong reg;

#ifdef OM_PIN
	if(OM_PIN == SDMMC_CHANNEL0) {
		/* 7 pins will be assigned - GPG0[0:6] = CLK, CMD, DAT[0:3], CDn */
		reg = readl(GPGCON) & 0xf0000000;
		writel(reg | 0x02222222, GPGCON);
		reg = readl(GPGPUD) & 0xffffc000;
		writel(reg | 0x00002000, GPGPUD);
		return ;
	} else if (OM_PIN == SDMMC_CHANNEL1) {
		/* 7 pins will be assigned - GPG0[0:6] = CLK, CMD, DAT[0:3], CDn */
		reg = readl(GPHCON0) & 0xff000000;
		writel(reg | 0x00222222, GPHCON0);
		reg = readl(GPHPUD) & 0xfffffc00;
		writel(reg, GPHPUD);
		return ;
	}
#endif

#ifdef USE_MMC0
	/* 7 pins will be assigned - GPG0[0:6] = CLK, CMD, DAT[0:3], CDn */
	reg = readl(GPGCON) & 0xf0000000;
	writel(reg | 0x02222222, GPGCON);
	reg = readl(GPGPUD) & 0xffffc000;
	writel(reg | 0x00002000, GPGPUD);
#endif

#ifdef USE_MMC1
	/* 7 pins will be assigned - GPG0[0:6] = CLK, CMD, DAT[0:3], CDn */
	reg = readl(GPHCON0) & 0xff000000;
	writel(reg | 0x00222222, GPHCON0);
	reg = readl(GPHPUD) & 0xfffffc00;
	writel(reg, GPHPUD);
#endif

#ifdef USE_MMC2
	/* 7 pins will be assigned - GPG0[0:6] = CLK, CMD, DAT[0:3], CDn */
	reg = readl(GPCCON) & 0xff00ffff;
	writel(reg | 0x00330000, GPCCON);
	reg = readl(GPHCON0) & 0x00ffffff;
	writel(reg | 0x33000000, GPCCON);
	reg = readl(GPHCON1) & 0xff;
	writel(reg | 0x33, GPCCON);


	reg = readl(GPCPUD) & 0xfffff0ff;
	writel(reg, GPCPUD);
	reg = readl(GPCPUD) & 0xfffc0fff;
	writel(reg, GPCPUD);
#endif
}

void setup_sdhci0_cfg_card(struct sdhci_host *host)
{
	u32 ctrl2;
	u32 ctrl3;

	/* don't need to alter anything acording to card-type */
	writel(S3C64XX_SDHCI_CONTROL4_DRIVE_9mA, host->ioaddr + S3C64XX_SDHCI_CONTROL4);

	ctrl2 = readl(host->ioaddr + S3C_SDHCI_CONTROL2);

	ctrl2 |= (S3C64XX_SDHCI_CTRL2_ENSTAASYNCCLR |
		  S3C64XX_SDHCI_CTRL2_ENCMDCNFMSK |
		  S3C_SDHCI_CTRL2_ENFBCLKRX |		  
		  S3C_SDHCI_CTRL2_DFCNT_NONE |
		  S3C_SDHCI_CTRL2_ENCLKOUTHOLD);

	/*
	if(host->mmc->clock == 52*1000000)
		ctrl3 = 0;
	else
		ctrl3 = S3C_SDHCI_CTRL3_FCSEL0 | S3C_SDHCI_CTRL3_FCSEL1;
	*/
	ctrl3 = 0x7f5f3f1f;

	writel(ctrl2, host->ioaddr + S3C_SDHCI_CONTROL2);
	writel(ctrl3, host->ioaddr + S3C_SDHCI_CONTROL3);
}

