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

	/* MMC0 clock src = SCLKMPLL */
	tmp = CLK_SRC4_REG & ~(0x0000000f);
	CLK_SRC4_REG = tmp | 0x00000006;

	/* MMC0 clock div */
	tmp = CLK_DIV4_REG & ~(0x0000000f);
	clock = get_MPLL_CLK()/1000000;
	for(i=0; i<0xf; i++)
	{
		if((clock / (i+1)) <= 50) {
			CLK_DIV4_REG = tmp | i<<0;
			break;
		}
	}

#ifdef USE_MMC1
	/* MMC1 clock src = SCLKMPLL */
	tmp = CLK_SRC4_REG & ~(0x000000f0);
	CLK_SRC4_REG = tmp | 0x00000060;

	/* MMC1 clock div */
	tmp = CLK_DIV4_REG & ~(0x000000f0);
	CLK_DIV4_REG = tmp | i<<4;
#endif	

#ifdef USE_MMC2
	/* MMC2 clock src = SCLKMPLL */
	tmp = CLK_SRC4_REG & ~(0x00000f00);
	CLK_SRC4_REG = tmp | 0x00000600;

	/* MMC2 clock div */
	tmp = CLK_DIV4_REG & ~(0x00000f00);
	CLK_DIV4_REG = tmp | i<<8;
#endif

#ifdef USE_MMC3
	/* MMC3 clock src = SCLKMPLL */
	tmp = CLK_SRC4_REG & ~(0x00000f00);
	CLK_SRC4_REG = tmp | 0x00000600;

	/* MMC3 clock div */
	tmp = CLK_DIV4_REG & ~(0x00000f00);
	CLK_DIV4_REG = tmp | i<<12;
#endif	
}

/*
 * this will set the GPIO for hsmmc ch0
 * GPG0[0:6] = CLK, CMD, CDn, DAT[0:3]
 */
void setup_hsmmc_cfg_gpio(void)
{
	ulong reg;

	/* MMC channel 0 */
	/* 7 pins will be assigned - GPG0[0:6] = CLK, CMD, CDn, DAT[0:3] */
	reg = readl(GPG0CON) & 0xf0000000;
	writel(reg | 0x02222222, GPG0CON);
	reg = readl(GPG0PUD) & 0xffffc000;
	writel(reg | 0x00002aaa, GPG0PUD);
	writel(0x00003fff, GPG0DRV);
#ifdef USE_MMC0_8BIT
	reg = readl(GPG1CON) & 0xf0000fff;
	writel(reg | 0x03333000, GPG1CON);
	reg = readl(GPG1PUD) & 0xffffc03f;
	writel(reg | 0x00002a80, GPG1PUD);
	writel(0x00003fc0, GPG1DRV);
#endif

#ifdef USE_MMC1
	/* MMC channel 1 */
	/* 7 pins will be assigned - GPG1[0:6] = CLK, CMD, CDn, DAT[0:3] */
	reg = readl(GPG1CON) & 0xf0000000;
	writel(reg | 0x02222222, GPG1CON);
	reg = readl(GPG1PUD) & 0xffffc000;
	writel(reg | 0x00002aaa, GPG1PUD);
	writel(0x00003fff, GPG1DRV);
#endif

#ifdef USE_MMC2
	/* MMC channel 2 */
	/* 7 pins will be assigned - GPG2[0:6] = CLK, CMD, CDn, DAT[0:3] */
	reg = readl(GPG2CON) & 0xf0000000;
	writel(reg | 0x02222222, GPG2CON);
	reg = readl(GPG2PUD) & 0xffffc000;
	writel(reg | 0x00002aaa, GPG2PUD);
	writel(0x00003fff, GPG2DRV);
#ifdef USE_MMC2_8BIT
	/* 4 pins will be assigned - GPG3[3:6] = DAT[4:7] */
	reg = readl(GPG3CON) & 0xf0000fff;
	writel(reg | 0x03333000, GPG3CON);
	reg = readl(GPG3PUD) & 0xffffc03f;
	writel(reg | 0x00002a80, GPG3PUD);
	writel(0x00003fc0, GPG3DRV);
#endif
#endif

#ifdef USE_MMC3
	/* MMC channel 3 */
	/* 7 pins will be assigned - GPG0[0:6] = CLK, CMD, CDn, DAT[0:3] */
	reg = readl(GPG3CON) & 0xf0000000;
	writel(reg | 0x02222222, GPG3CON);
	reg = readl(GPG3PUD) & 0xffffc000;
	writel(reg | 0x00002aaa, GPG3PUD);
	writel(0x00003fff, GPG3DRV);
#endif
}

#define S3C_SDHCI_CTRL3_FCSELTX_INVERT  (0)
#define S3C_SDHCI_CTRL3_FCSELTX_BASIC \
	(S3C_SDHCI_CTRL3_FCSEL3 | S3C_SDHCI_CTRL3_FCSEL2)
#define S3C_SDHCI_CTRL3_FCSELRX_INVERT  (0)
#define S3C_SDHCI_CTRL3_FCSELRX_BASIC \
	(S3C_SDHCI_CTRL3_FCSEL1 | S3C_SDHCI_CTRL3_FCSEL0)

void setup_sdhci0_cfg_card(struct sdhci_host *host)
{
	u32 ctrl2;
	u32 ctrl3;

	/* don't need to alter anything acording to card-type */
	writel(S3C64XX_SDHCI_CONTROL4_DRIVE_9mA, host->ioaddr + S3C64XX_SDHCI_CONTROL4);

	ctrl2 = readl(host->ioaddr + S3C_SDHCI_CONTROL2);

	ctrl2 |= (S3C64XX_SDHCI_CTRL2_ENSTAASYNCCLR |
		  S3C64XX_SDHCI_CTRL2_ENCMDCNFMSK |
		  S3C_SDHCI_CTRL2_DFCNT_NONE |
		  S3C_SDHCI_CTRL2_ENCLKOUTHOLD);
	ctrl3 = 0;

	if (host->clock <= 400000) {
		ctrl2 &= ~(S3C_SDHCI_CTRL2_ENFBCLKTX |
			S3C_SDHCI_CTRL2_ENFBCLKRX);
		ctrl3 = 0;
	} else if (host->clock > 20000000 && host->clock < 52000000) {
		ctrl2 |= S3C_SDHCI_CTRL2_ENFBCLKTX |
			S3C_SDHCI_CTRL2_ENFBCLKRX;
		ctrl3 = S3C_SDHCI_CTRL3_FCSELTX_BASIC |
			S3C_SDHCI_CTRL3_FCSELRX_BASIC;
	}
	else {
		ctrl2 |= S3C_SDHCI_CTRL2_ENFBCLKTX |
			S3C_SDHCI_CTRL2_ENFBCLKRX;
		ctrl3 = S3C_SDHCI_CTRL3_FCSELTX_BASIC |
			S3C_SDHCI_CTRL3_FCSELRX_INVERT;
	}

	writel(ctrl2, host->ioaddr + S3C_SDHCI_CONTROL2);
	writel(ctrl3, host->ioaddr + S3C_SDHCI_CONTROL3);
}

