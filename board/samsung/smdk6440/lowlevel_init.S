/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *                     Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 * Modified for the Samsung SMDK2410 by
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
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


#include <config.h>
#include <version.h>

#include <s5p6440.h>
#include "smdk6440_val.h"

_TEXT_BASE:
	.word	TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	mov	r12, lr

	/* Disable Watchdog */
	ldr	r0, =0xEA200000
	mov	r1, #0
	str	r1, [r0]

	ldr	r0, =ELFIN_GPIO_BASE
#if 0
	ldr	r1, =0x05555555
#else
	ldr	r1, =0x0
#endif
	str	r1, [r0, #MEM1DRVCON_OFFSET]

	@ External interrupt pending clear
	ldr	r0, =(ELFIN_GPIO_BASE+EINTPEND_OFFSET)	/*EINTPEND*/
	ldr	r1, [r0]
	str	r1, [r0]

	ldr	r0, =ELFIN_VIC0_BASE_ADDR 	@0x71200000
	ldr	r1, =ELFIN_VIC1_BASE_ADDR 	@0x71300000

	@ Disable all interrupts (VIC0 and VIC1)
	mvn	r3, #0x0
	str	r3, [r0, #oINTMSK]
	str	r3, [r1, #oINTMSK]

	@ Set all interrupts as IRQ
	mov	r3, #0x0
	str	r3, [r0, #oINTMOD]
	str	r3, [r1, #oINTMOD]

	@ Pending Interrupt Clear
	mov	r3, #0x0
	str	r3, [r0, #oVECTADDR]
	str	r3, [r1, #oVECTADDR]

#if 1
	ldr	r0, =0xff000fff
	bic	r1, pc, r0		/* r0 <- current base addr of code */
	ldr	r2, _TEXT_BASE		/* r1 <- original base addr in ram */
	bic	r2, r2, r0		/* r0 <- current base addr of code */
	cmp 	r1, r2			/* compare r0, r1                  */
	beq	1f			/* r0 == r1 then skip sdram init   */
#endif
	/* init system clock */
	bl 	system_clock_init
	
	bl	mem_ctrl_asm_init

1:
	/* for UART */
	bl 	uart_asm_init

#if 1
        ldr     r0, =(ELFIN_CLOCK_POWER_BASE+RST_STAT_OFFSET)
        ldr     r1, [r0]
        bic     r1, r1, #0xfffffff7
        cmp     r1, #0x8
        beq     wakeup_reset

#endif

	ldr	r0, =ELFIN_UART_BASE
	ldr	r1, =0x4b4b4b4b
	str	r1, [r0, #UTXH_OFFSET]

	mov	lr, r12
	mov	pc, lr
#if 1
wakeup_reset:

	/*Clear wakeup status register*/
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE+WAKEUP_STAT_OFFSET)
	ldr	r1, [r0]
	str	r1, [r0]

        /*LED test*/
        ldr     r0, =ELFIN_GPIO_BASE
        ldr     r1, =0x3000
        str     r1, [r0, #GPNDAT_OFFSET]

	/*Load return address and jump to kernel*/
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE+INF_REG0_OFFSET)
	ldr	r1, [r0]	/* r1 = physical address of s3c6400_cpu_resume function*/
	mov	pc, r1		/*Jump to kernel (sleep-s3c6400.S)*/
	nop
	nop
#endif
/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
system_clock_init:

#ifdef SMDK6440_DDR2
	ldr	r0, =ELFIN_CLOCK_POWER_BASE	@0xE0100000

	/* APLL MPLL Clock select*/
	mov	r1, #0
	orr	r1, r1, #VSEL_M_ENABLE
	str	r1, [r0, #CLK_SRC0_OFFSET]

	ldr	r1, [r0, #CLK_SRC1_OFFSET]
	ldr	r2, =0x200
	str	r2, [r0, #CLK_SRC1_OFFSET]

	ldr	r1, [r0, #OTHERS_OFFSET]
	ldr	r2, =0x00018F1E
	str	r2, [r0, #OTHERS_OFFSET]

	mov	r1, #0xe10
	str	r1, [r0, #APLL_LOCK_OFFSET]
	str	r1, [r0, #MPLL_LOCK_OFFSET]

	ldr   	r1, [r0, #CLK_DIV0_OFFSET]	/*Set Clock Divider 0 */
	bic	r1, r1, #0xff00
	bic	r1, r1, #0x1f
	ldr	r2, =CLK_DIV_VAL
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV0_OFFSET]

	ldr   	r1, [r0, #CLK_DIV3_OFFSET]	/*Set Clock Divider 3 */
	bic	r1, r1, #0xff00
	bic	r1, r1, #0x1f
	ldr	r2, =CLK_DIV_VAL
	orr	r2, r2, #0x10
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV3_OFFSET]

	ldr	r1, [r0, #CLK_DIV1_OFFSET]
	ldr	r2, =0x00000000
	str	r2, [r0, #CLK_DIV1_OFFSET]

	ldr	r1, [r0, #CLK_DIV2_OFFSET]
	ldr	r2, =0x00000000
	str	r2, [r0, #CLK_DIV2_OFFSET]

	ldr	r1, =APLL_VAL
	str	r1, [r0, #APLL_CON_OFFSET]
	ldr	r1, =MPLL_VAL
	str	r1, [r0, #MPLL_CON_OFFSET]

	/* wait at least 200us to stablize all clock */
	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

	ldr	r1, [r0, #CLK_SRC0_OFFSET]	/* APLL, MPLL, EPLL select to Fout */
	orr	r1, r1, #APLL_SEL_ENABLE
	orr	r1, r1, #MPLL_SEL_ENABLE
	orr	r1, r1, #VSEL_M_ENABLE
	str	r1, [r0, #CLK_SRC0_OFFSET]

	mov	pc, lr
	
#else

	ldr	r0, =ELFIN_CLOCK_POWER_BASE	@0xE0100000

	/* APLL MPLL Clock select*/
	mov	r1, #0
	orr	r1, r1, #VSEL_M_ENABLE
	orr	r1, r1, #MMC1_SEL_DOUT_ENABLE
	str	r1, [r0, #CLK_SRC0_OFFSET]

	ldr   	r1, [r0, #CLK_DIV0_OFFSET]	/*Set Clock Divider 0 */
	bic	r1, r1, #0xff00
	bic	r1, r1, #0x1f
	ldr	r2, =CLK_DIV_VAL
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV0_OFFSET]

	ldr   	r1, [r0, #CLK_DIV3_OFFSET]	/*Set Clock Divider 3 */
	bic	r1, r1, #0xff00
	bic	r1, r1, #0x1f
	ldr	r2, =CLK_DIV_VAL
	orr	r2, r2, #0x10
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV3_OFFSET]

	mov	r1, #0xe10
	str	r1, [r0, #APLL_LOCK_OFFSET]
	str	r1, [r0, #MPLL_LOCK_OFFSET]

	ldr	r1, =APLL_VAL
	str	r1, [r0, #APLL_CON_OFFSET]
	ldr	r1, =MPLL_VAL
	str	r1, [r0, #MPLL_CON_OFFSET]

	/* wait at least 200us to stablize all clock */
	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

	ldr	r1, [r0, #CLK_SRC0_OFFSET]	/* APLL, MPLL, EPLL select to Fout */
	orr	r1, r1, #APLL_SEL_ENABLE
	orr	r1, r1, #MPLL_SEL_ENABLE
	orr	r1, r1, #VSEL_M_ENABLE
	orr	r1, r1, #MMC1_SEL_DOUT_ENABLE
	orr	r1, r1, #MMC0_SEL_DOUT_ENABLE
	str	r1, [r0, #CLK_SRC0_OFFSET]

	mov	pc, lr
#endif	

/*
 * uart_asm_init: Initialize UART in asm mode, 115200bps fixed.
 * void uart_asm_init(void)
 */
uart_asm_init:
	/* set GPIO to enable UART */
	@ GPIO setting for UART
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x222222
	str   	r1, [r0, #GPACON_OFFSET]

	ldr	r0, =ELFIN_UART_CONSOLE_BASE		@0x7F005000
	mov	r1, #0x0
	str	r1, [r0, #UFCON_OFFSET]
	str	r1, [r0, #UMCON_OFFSET]

	mov	r1, #0x3                	@was 0.
	str	r1, [r0, #ULCON_OFFSET]

	ldr	r1, =0x245			/* UARTCLK SRC = x0 => PCLK */
	str	r1, [r0, #UCON_OFFSET]

#if defined(CONFIG_CLK_667_166_83)
	ldr	r1, =0x2c
	str	r1, [r0, #UBRDIV_OFFSET]

	ldr	r1, =0x0888

	str	r1, [r0, #UDIVSLOT_OFFSET]
#elif defined(CONFIG_CLK_533_133_66)
	ldr	r1, =0x22
	str	r1, [r0, #UBRDIV_OFFSET]

	ldr	r1, =0xDDDD

	str	r1, [r0, #UDIVSLOT_OFFSET]
#elif defined(CONFIG_CLK_400_100_50)
	ldr	r1, =0x1A
	str	r1, [r0, #UBRDIV_OFFSET]

	ldr	r1, =0x0808

	str	r1, [r0, #UDIVSLOT_OFFSET]
#endif

	ldr	r1, =0x4f4f4f4f
	str	r1, [r0, #UTXH_OFFSET]		@'O'

	mov	pc, lr

/*
 * Nand Interface Init for SMDK6400 */
.globl nand_asm_init
nand_asm_init:

	ldr	r5, =ELFIN_NAND_BASE
	ldr	r6, [r5, #NFCONF_OFFSET]
	orr	r6, r6, #0x70
	orr	r6, r6, #0x7700
	str     r6, [r5, #NFCONF_OFFSET]

	ldr	r6, [r5, #NFCONT_OFFSET]
	orr	r6, r6, #0x03
	str     r6, [r5, #NFCONT_OFFSET]

	mov	pc, lr

#ifdef CONFIG_ENABLE_MMU

/*
 * MMU Table for SMDK6440
 */

	/* form a first-level section entry */
.macro FL_SECTION_ENTRY base,ap,d,c,b
	.word (\base << 20) | (\ap << 10) | \
	      (\d << 5) | (1<<4) | (\c << 3) | (\b << 2) | (1<<1)
.endm
.section .mmudata, "a"
	.align 14
	// the following alignment creates the mmu table at address 0x4000.
	.globl mmu_table
mmu_table:
	.set __base,0
	// 1:1 mapping for debugging
	.rept 0xA00
	FL_SECTION_ENTRY __base,3,0,0,0
	.set __base,__base+1
	.endr

	//.rept 0x1000
	//FL_SECTION_ENTRY __base,3,0,0,0
	//.set __base,__base+1
	//.endr

	// access is not allowed.
	.rept 0xC00 - 0xA00
	.word 0x00000000
	.endr

	// 256MB for SDRAM 0xC0000000 -> 0x20000000
	.set __base, 0x200
	.rept 0xD00 - 0xC00
	FL_SECTION_ENTRY __base,3,0,1,1
	.set __base,__base+1
	.endr

	// access is not allowed.
	//.rept 0xD00 - 0xC80
	//.word 0x00000000
	//.endr

	.set __base, 0xD00
	// 1:1 mapping for debugging
	.rept 0x1000 - 0xD00
	FL_SECTION_ENTRY __base,3,0,0,0
	.set __base,__base+1
	.endr

	// access is not allowed.
	//.rept 0x1000 - 0xc80
	//.word 0x00000000
	//.endr

#endif

