/*
 * $Id: nand_cp.c,v 1.1 2008/11/20 01:08:36 boyko Exp $
 *
 * (C) Copyright 2006 Samsung Electronics
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

/*
 * You must make sure that all functions in this file are designed
 * to load only U-Boot image.
 *
 * So, DO NOT USE in common read.
 *
 * By scsuh.
 */


#include <common.h>

#ifdef CONFIG_S5PC1XX
#include <asm/io.h>
#include <linux/mtd/nand.h>
#include <regs.h>

#define NAND_CONTROL_ENABLE()	(NFCONT_REG |= (1 << 0))

/*
 * address format
 *              17 16         9 8            0
 * --------------------------------------------
 * | block(12bit) | page(5bit) | offset(9bit) |
 * --------------------------------------------
 */

static int nandll_read_page (uchar *buf, ulong addr, int large_block, int page_size)
{
        int i;

        NAND_ENABLE_CE();

        NFCMD_REG = NAND_CMD_READ0;

        /* Write Address */
        NFADDR_REG = 0;

	if (large_block)
	        NFADDR_REG = 0;

	NFADDR_REG = (addr) & 0xff;
	NFADDR_REG = (addr >> 8) & 0xff;
	NFADDR_REG = (addr >> 16) & 0xff;

	if (large_block)
		NFCMD_REG = NAND_CMD_READSTART;

        NF_TRANSRnB();

	/* for compatibility(2460). u32 cannot be used. by scsuh */
	for(i=0; i < page_size; i++) {
                *buf++ = NFDATA8_REG;
        }

        NAND_DISABLE_CE();
        return 0;
}

/*
 * Read data from NAND.
 */
static int nandll_read_blocks (ulong dst_addr, ulong size, int large_block, int page_shift)
{
        uchar *buf = (uchar *)dst_addr;
        int i;

        /* Read pages */
        for (i = 0; i < (size>>page_shift); i++, buf+=(1<<page_shift)) {
                nandll_read_page(buf, i, large_block, 1 << page_shift);
        }

        return 0;
}

int copy_uboot_to_ram (void)
{
	int large_block = 0;
	int i;
	vu_char id;
	int page_shift = 9;

	NAND_CONTROL_ENABLE();
        NAND_ENABLE_CE();
        NFCMD_REG = NAND_CMD_READID;
        NFADDR_REG =  0x00;

	/* wait for a while */
        for (i=0; i<200; i++);
	id = NFDATA8_REG;
	id = NFDATA8_REG;

	if (id > 0x80) {
		large_block = 1;
		id = NFDATA8_REG;
		id = NFDATA8_REG;
		page_shift = 10+(id&3);
	}

	/* read NAND Block.
	 * read the size of UBOOT to RAM
	 */
	return nandll_read_blocks(CFG_PHY_UBOOT_BASE, COPY_BL2_SIZE, large_block, page_shift);
}

void ltc_init_gpio_bit(char bit, char value )
{
	int temp;

	temp = __raw_readl(GPH0CON);

	if (value == 1)
		temp |= (1 << (bit * 4));
	else
		temp &= ~(1 << (bit * 4));

	__raw_writel(temp,GPH0CON);
}

void ltc_set_gpio_bit(char bit, char value )
{
	int temp;

	temp = __raw_readl(GPH0DAT);

	if (value == 1)
		temp |= (1 << bit);
	else
		temp &= ~(1 << bit);

	__raw_writel(temp,GPH0DAT);
}

void ltc_pull_gpio_bit(char bit, char value )
{
	int temp;

	temp = __raw_readl(GPH0PUD);

	if (value == 1)
		temp |= (1 << (bit * 2));
	else
		temp &= ~(1 << (bit * 2));

	__raw_writel(temp,GPH0PUD);
}

#define SER_BIT 0
#define SRCLK_BIT 1
#define RCLK_ARM_BIT 2
#define RCLK_INT_BIT 3
#define VID_CTRL_BIT 4

static inline void ltc_delay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n" "bne 1b":"=r" (loops):"0"(loops));
}

void set_ltc_for_833(void)
{
	int iter;
	int temp;
/*
1.00 = 0x0f
1.05 = 0x0e
1.10 = 0x0d
1.15 = 0x0c
1.20 = 0x0b
1.25 = 0x0a
1.30 = 0x09
1.35 = 0x08
1.40 = 0x07
1.45 = 0x06
1.50 = 0x05
*/

	int value_for_voltage = 0x08;
	/*init GPIO for LTC*/

	ltc_set_gpio_bit(RCLK_ARM_BIT, 0);
	ltc_init_gpio_bit(RCLK_ARM_BIT, 1);
	ltc_pull_gpio_bit(RCLK_ARM_BIT,0);

	ltc_set_gpio_bit(RCLK_INT_BIT, 0);
	ltc_init_gpio_bit(RCLK_INT_BIT, 1);
	ltc_pull_gpio_bit(RCLK_INT_BIT,0);

	ltc_set_gpio_bit(SER_BIT, 0);
	ltc_init_gpio_bit(SER_BIT, 1);
	ltc_pull_gpio_bit(SER_BIT,0);

	ltc_set_gpio_bit(SRCLK_BIT, 0);
	ltc_init_gpio_bit(SRCLK_BIT, 1);
	ltc_pull_gpio_bit(SRCLK_BIT,0);

	ltc_set_gpio_bit(VID_CTRL_BIT, 1);
	ltc_init_gpio_bit(VID_CTRL_BIT, 1);
	ltc_pull_gpio_bit(VID_CTRL_BIT,0);

	ltc_delay(100);

	/* setting Regulator */

	/* serial shifter setting */
	ltc_set_gpio_bit(RCLK_ARM_BIT,0);
	ltc_set_gpio_bit(RCLK_INT_BIT,0);

	for (iter = 8 ; iter > 0 ; iter-- ){
		ltc_set_gpio_bit(SRCLK_BIT, 0);
		ltc_set_gpio_bit(SER_BIT, (value_for_voltage>>(iter-1))&0x1);
		ltc_delay(40000);
		ltc_set_gpio_bit(SRCLK_BIT, 1);
		ltc_delay(40000);
	}

	ltc_set_gpio_bit(SRCLK_BIT, 0);
	ltc_set_gpio_bit(SER_BIT, 0);
	ltc_delay(40000);

	/* serial shifter output enable */
	ltc_set_gpio_bit(VID_CTRL_BIT, 0);
	ltc_set_gpio_bit(RCLK_ARM_BIT,1);
	ltc_set_gpio_bit(RCLK_INT_BIT,1);
	ltc_delay(40000);
	ltc_set_gpio_bit(RCLK_ARM_BIT,0);
	ltc_set_gpio_bit(RCLK_INT_BIT,1);
	ltc_delay(40000);

}

void temp_uart(void)
{
	int tmp, i;

	tmp = __raw_readl(ELFIN_CLOCK_POWER_BASE+RST_STAT_OFFSET);

	serial_putc('\n');

	for(i = 31 ; i >= 0 ; i-- ){
		if ((tmp >> i) & 0x1)
			serial_putc('1');
		else
			serial_putc('0');

		if ((i % 4) == 0)
			serial_putc('|');
	}

	serial_putc('\n');

}

#endif

