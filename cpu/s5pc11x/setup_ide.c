/* u-boot/board/samsung/smdkc110/ide.h
 *
 * Copyright (C) 2009 Samsung Electronics
 *      http://samsungsemi.com/
 *
 * Header file for IDE support on SMDKV210
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

#include <asm/io.h>
#include <ata.h>
#include <s5pc110.h>

/* Delay Function */
#define mdelay(n) ({unsigned long msec=(n); \
			while (msec--) udelay(1000);})

/* V210 CF-ATA IO Register Description */
#define ELFIN_ATA_BASE	0xE8200000
#define ATA_CONTROL	(ELFIN_ATA_BASE + 0x00)
#define ATA_COMMAND	(ELFIN_ATA_BASE + 0x08)
#define ATA_SWRST	(ELFIN_ATA_BASE + 0x0c)
#define ATA_IRQ		(ELFIN_ATA_BASE + 0x10)
#define ATA_IRQ_MASK	(ELFIN_ATA_BASE + 0x14)
#define ATA_CFG		(ELFIN_ATA_BASE + 0x18)


static void set_ata_enable(unsigned int on)
{
	unsigned int temp;

	temp = __raw_readl(ATA_CONTROL);
	if (on)
		__raw_writel(temp | 0x1, ATA_CONTROL);
	else
		__raw_writel(temp & (~0x1), ATA_CONTROL);
}

static void set_endian_mode(unsigned int little)
{
	unsigned int reg = __raw_readl(ATA_CFG);

	/* set Little endian */
	if (little)
		reg &= (~0x40);
	else
		reg |= 0x40;
	__raw_writel(reg, ATA_CFG);
}

int ide_preinit(void)
{
	unsigned int reg, tmp;

        /* CF_Add[0 - 2], CF_IORDY, CF_INTRQ, CF_DMARQ, CF_DMARST, CF_DMACK */
	 __raw_writel(0x44444444, GPJ0CON);
	__raw_writel(0x0, GPJ0PUD);
	__raw_writel(0xffff, GPJ0DRV);

        /*CF_Data[0 - 7] */
	__raw_writel(0x44444444, GPJ2CON);
	__raw_writel(0x0, GPJ2PUD);
	__raw_writel(0xffff, GPJ2DRV);
	
	/* CF_Data[8 - 15] */
	__raw_writel(0x44444444, GPJ3CON);
	__raw_writel(0x0, GPJ3PUD);
	__raw_writel(0xffff, GPJ3DRV_SR);
        
	/* CF_CS0, CF_CS1, CF_IORD, CF_IOWR */
	reg = __raw_readl(GPJ4CON) & ~(0xffff);
	__raw_writel(reg | 0x4444, GPJ4CON);
	reg = __raw_readl(GPJ4PUD) & ~(0xff);
	__raw_writel(0xff, GPJ4DRV);
	mdelay(100);
	
	set_endian_mode(0);
	set_ata_enable(1);
	mdelay(100);
	
	/* Remove IRQ Status */
	__raw_writel(0x3f, ATA_IRQ);
	__raw_writel(0x3f, ATA_IRQ_MASK);
	
	return (0);
}
