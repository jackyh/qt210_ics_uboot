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
		
	tmp = CLK_SRC2_REG & ~(0x00000fff);
	CLK_SRC2_REG = tmp | 0x00000111;
	tmp = CLK_DIV3_REG & ~(0x00000fff);

	clock = get_DoutMpll()/1000000;
	for(i=0; i<0xf; i++)
	{
		if((clock / (i+1)) <= 50) {
			CLK_DIV3_REG = tmp | i<<0 | i<<4 | i<<8;
			break;
		}
	}
}

/*
 * this will set the GPIO for hsmmc ch0
 * GPG0[0:5] = CLK, CMD, DAT[0:3]
 * GPG0[6:7] = DAT[4:5]
 * GPG1[0:1] = DAT[6:7]
 * GPG1[2]   = CD
 */
void setup_hsmmc0_cfg_gpio(void)
{
	ulong reg;

	/* 6 pins will be assigned. clk, cmd, dat[0:3] - GPG0[0:5] */
	reg = GPG0CON_REG & 0xff000000;
	GPG0CON_REG = reg | 0x00222222;
	reg = GPG0PUD_REG & 0xfffff000;
	GPG0PUD_REG = reg;
	GPG0DRV_REG = 0x00000fff;

	/* 1 pin will be assigned. cd[0] - GPG1[2] */
	reg = GPG1CON_REG & 0xfffff0ff;
	GPG1CON_REG = reg | 0x00000200;
	reg = GPG1PUD_REG & 0xffffffcf;
	GPG1PUD_REG = reg | 0x00000020;
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
		  S3C_SDHCI_CTRL2_DFCNT_NONE |
		  S3C_SDHCI_CTRL2_ENCLKOUTHOLD);

	ctrl3 = 0;

	writel(ctrl2, host->ioaddr + S3C_SDHCI_CONTROL2);
	writel(ctrl3, host->ioaddr + S3C_SDHCI_CONTROL3);
}

