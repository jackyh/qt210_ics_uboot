/*
 * (C) Copyright 2001-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
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

/* This code should work for both the S3C2400 and the S3C2410
 * as they seem to have the same PLL and clock machinery inside.
 * The different address mapping is handled by the s3c24xx.h files below.
 */

#include <common.h>
#include <s5pc100.h>

#define APLL 0
#define MPLL 1
#define EPLL 2


/* ------------------------------------------------------------------------- */
/* NOTE: This describes the proper use of this file.
 *
 * CONFIG_SYS_CLK_FREQ should be defined as the input frequency of the PLL.
 *
 * get_FCLK(), get_HCLK(), get_PCLK() and get_UCLK() return the clock of
 * the specified bus in HZ.
 */
/* ------------------------------------------------------------------------- */

static ulong get_PLLCLK(int pllreg)
{
	ulong r, m, p, s;

	if (pllreg == APLL)
		r = APLL_CON_REG;
	else if (pllreg == MPLL)
		r = MPLL_CON_REG;
	else if (pllreg == EPLL)
		r = EPLL_CON_REG;
	else
		hang();

	m = (r>>16) & 0x3ff;
	p = (r>>8) & 0x3f;
	s = r & 0x7;

	return (m * (CONFIG_SYS_CLK_FREQ / (p * (1 << s))));
}

/* return ARMCORE frequency */
ulong get_ARMCLK(void)
{
	ulong div,arm_div,apll_div;

	div = CLK_DIV0_REG;
	arm_div = ((div>>4) & 0x7) + 1;
	apll_div = (div&0x1) + 1;

	return ((get_PLLCLK(APLL)) / (arm_div*apll_div));

}

/* return FCLK frequency */
ulong get_FCLK(void)
{
	return (get_PLLCLK(APLL));
}

/* return HCLK frequency */
ulong get_HCLK(void)
{
	ulong fclk;

	uint div,div_apll, div_arm, div_d0_bus;

	div = CLK_DIV0_REG;

	div_apll = (div&0x1) + 1;
	div_arm = ((div>>4) & 0x7) + 1;
	div_d0_bus = ((div>>8) & 0x7) + 1;

	fclk = get_FCLK();

	return fclk/(div_apll * div_arm * div_d0_bus);
}

/* return PCLK frequency */
ulong get_PCLK(void)
{
	ulong fclk;
	uint div = CLK_DIV1_REG;
	uint div_d1_bus = ((div>>12) & 0x7) + 1;
	uint div_pclk = ((div>>16) & 0x7) + 1;

	fclk = get_PLLCLK(MPLL);	// ASYNC Mode

	return fclk/(div_d1_bus * div_pclk);
}

/* return DoutMpll frequency */
ulong get_DoutMpll(void)
{
	ulong fclk;
	uint div = CLK_DIV1_REG;
	uint div_mpll = ((div>>4) & 0x3) + 1;

	fclk = get_PLLCLK(MPLL);

	return fclk/div_mpll;
}

/* return UCLK frequency */
ulong get_UCLK(void)
{
	return (get_PLLCLK(EPLL));
}

int print_cpuinfo(void)
{
	printf("\nCPU:     S5PC100@%ldMHz\n", get_ARMCLK()/1000000);
	printf("         Fclk = %ldMHz, Hclk = %ldMHz, Pclk = %ldMHz, DoutMpll = %ldMHz",
			get_FCLK()/1000000, get_HCLK()/1000000, get_PCLK()/1000000, get_DoutMpll()/1000000);

/**************
* Display Serial SRC
***************/

#if defined(CONFIG_CLKSRC_CLKUART)
	puts(", Serial = CLKUART ");
#else
	puts(", Serial = PCLK ");
#endif

	return 0;
}

