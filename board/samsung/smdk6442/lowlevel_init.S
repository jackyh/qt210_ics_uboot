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

#include <s5p6442.h>
#include "smdk6442_val.h"

_TEXT_BASE:
	.word	TEXT_BASE

	.globl lowlevel_init
lowlevel_init:
	mov	r12, lr

	/* IO Retention release */
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE + OTHERS_OFFSET)
	ldr	r1, [r0]
	ldr	r2, =IO_RET_REL
	orr	r1, r1, r2
	str	r1, [r0]

	/* Disable Watchdog */
	ldr	r0, =ELFIN_WATCHDOG_BASE	
	mov	r1, #0
	str	r1, [r0]

#if 0
	/* SROMC Bus Control for BANK0 */
	ldr	r0, =ELFIN_SROM_BASE	/* 0xE7000000 */
	mov	r1, #0xb
	str	r1, [r0]

	/* when BUS = 12MHz */
	mov	r1, #0x0
	str	r1, [r0, #0x4]

	/* GPJ0 SROM_ADDR_16~20 set 20 set */
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x3333333
	str	r1, [r0, #GPJ0CON_OFFSET]
#endif
	/* SROMC Bus Control for BANK0 */
	ldr	r0, =ELFIN_SROM_BASE	/* 0xE7000000 */
	mov	r1, #0xb8
	str	r1, [r0]

	/* GPJ0 SROM_ADDR_16~20 set 20 set */
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x3333333
	str	r1, [r0, #GPJ0CON_OFFSET]

	/* when we already run in ram, we don't need to relocate U-Boot.
	 * and actually, memory controller must be configured before U-Boot
	 * is running in ram.
	 */
	ldr	r0, =0xff000fff
	bic	r1, pc, r0		/* r0 <- current base addr of code */
	ldr	r2, _TEXT_BASE		/* r1 <- original base addr in ram */
	bic	r2, r2, r0		/* r0 <- current base addr of code */
	cmp     r1, r2                  /* compare r0, r1                  */
	beq     1f			/* r0 == r1 then skip sdram init   */

	/* init system clock */
	bl system_clock_init
	bl mem_ctrl_asm_init

1:
	/* for UART */
	bl uart_asm_init
	ldr     r0, =(ELFIN_CLOCK_POWER_BASE+RST_STAT_OFFSET)
        ldr     r1, [r0]
        bic     r1, r1, #0xfffeffff
        ldr		r2, =0x10000
        cmp     r1, r2
        beq     wakeup_reset

	ldr	r0, =ELFIN_UART_BASE
	ldr	r1, =0x4b4b4b4b
	str	r1, [r0, #UTXH_OFFSET]

	mov	pc, r12

#ifndef FPGA_SMDK6442
wakeup_reset:
	/* all leds on for test */
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x40
	str	r1, [r0, #GPH1DAT_OFFSET]

	/*Clear wakeup status register*/
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE+WAKEUP_STAT_OFFSET)
	ldr	r1, [r0]
	str	r1, [r0]

	/*Load return address and jump to kernel*/
	ldr	r0, =(INF_REG_BASE+INF_REG0_OFFSET)
	ldr	r1, [r0]	/* r1 = physical address of s3c6400_cpu_resume function*/

	mov	pc, r1		/*Jump to kernel (sleep-s3c6400.S)*/
	nop
	nop
#endif

#ifndef FPGA_SMDK6442
/*
 * system_clock_init: Initialize core clock and bus clock.
 * void system_clock_init(void)
 */
system_clock_init:

	ldr	r0, =ELFIN_CLOCK_POWER_BASE	@0xE0100000

	/* Set Mux to FIN */
	ldr	r1, =0x0
	str	r1, [r0, #CLK_SRC0_OFFSET]

	/* Disable PLL */
	ldr	r1, =0x0
	str	r1, [r0, #APLL_CON_OFFSET]
	ldr	r1, =0x0
	str	r1, [r0, #MPLL_CON_OFFSET]
	ldr	r1, =0x0
	str	r1, [r0, #VPLL_CON_OFFSET]
	 
#ifdef CONFIG_SMDK6442_EVT1
	ldr	r1, =0x829B0601
	str	r1, [r0, #APLL_CON_OFFSET]

	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

 	ldr	r1, =0x810A0303
	str	r1, [r0, #MPLL_CON_OFFSET]

	mov	r1, #0x10000
2:	subs	r1, r1, #1
	bne	2b

	ldr	r1, =0x80200103
	str	r1, [r0, #EPLL_CON_OFFSET]

	mov	r1, #0x10000
3:	subs	r1, r1, #1
	bne	3b

	ldr	r1, =0x806C0303
	str	r1, [r0, #VPLL_CON_OFFSET]
#else
	ldr	r1, =APLL_VAL
	str	r1, [r0, #APLL_CON_OFFSET]

	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

 	ldr	r1, =MPLL_VAL
	str	r1, [r0, #MPLL_CON_OFFSET]

	mov	r1, #0x10000
2:	subs	r1, r1, #1
	bne	2b

	ldr	r1, =EPLL_VAL
	str	r1, [r0, #EPLL_CON_OFFSET]

	mov	r1, #0x1000
3:	subs	r1, r1, #1
	bne	3b

	ldr	r1, =VPLL_VAL
	str	r1, [r0, #VPLL_CON_OFFSET]
#endif

	mov	r1, #0x10000
4:	subs	r1, r1, #1
	bne	4b
	
	ldr	r1, [r0, #CLK_SRC0_OFFSET]
	ldr	r2, =0x1111
	orr	r1, r1, r2
	str	r1, [r0, #CLK_SRC0_OFFSET]

	ldr	r1, =0x10100000
	str	r1, [r0, #CLK_DIV0_OFFSET]
	
	mov	pc, lr
#endif

/*
 * uart_asm_init: Initialize UART in asm mode, 115200bps fixed.
 * void uart_asm_init(void)
 */
uart_asm_init:

/*	ldr	r1, =0x1111
	ldr	r2, =CLK_SRC4_VAL
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV0_OFFSET]
*/
	/* set GPIO(GPA) to enable UART */
	@ GPIO setting for UART
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x22222222
	str   	r1, [r0, #GPA0CON_OFFSET]

	ldr     r1, =0x2222
	str     r1, [r0, #GPA1CON_OFFSET]

	ldr	r0, =ELFIN_UART_CONSOLE_BASE		@0xEC000000
	mov	r1, #0x0
	str	r1, [r0, #UFCON_OFFSET]
	str	r1, [r0, #UMCON_OFFSET]

	mov	r1, #0x3
	str	r1, [r0, #ULCON_OFFSET]

	ldr	r1, =0x3c5
	str	r1, [r0, #UCON_OFFSET]

	ldr	r1, =UART_UBRDIV_VAL
	str	r1, [r0, #UBRDIV_OFFSET]

	ldr	r1, =UART_UDIVSLOT_VAL
	str	r1, [r0, #UDIVSLOT_OFFSET]

	ldr	r1, =0x4f4f4f4f
	str	r1, [r0, #UTXH_OFFSET]		@'O'

	mov	pc, lr

#ifdef CONFIG_ENABLE_MMU

/*
 * MMU Table for SMDKC100
 * 0x0000_0000 -- 0xBFFF_FFFF => Not Allowed
 * 0xB000_0000 -- 0xB7FF_FFFF => A:0xB000_0000 -- 0xB7FF_FFFF
 * 0xC000_0000 -- 0xC7FF_FFFF => A:0x2000_0000 -- 0x27FF_FFFF
 * 0xC800_0000 -- 0xDFFF_FFFF => Not Allowed
 * 0xE000_0000 -- 0xFFFF_FFFF => A:0xE000_0000 -- 0XFFFF_FFFF
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
	// Access for iRAM
	.rept 0x100
	FL_SECTION_ENTRY __base,3,0,0,0
	.set __base,__base+1
	.endr

	// Not Allowed
	.rept 0x200 - 0x100
	.word 0x00000000
	.endr

	.set __base,0x200
	// should be accessed
	.rept 0x300 - 0x200
	FL_SECTION_ENTRY __base,3,0,1,1
	.set __base,__base+1
	.endr

	.rept 0x800 - 0x300
	.word 0x00000000
	.endr

	.set __base,0x800
	// should be accessed
	.rept 0xb00 - 0x800
	FL_SECTION_ENTRY __base,3,0,0,0
	.set __base,__base+1
	.endr

/*	.rept 0xc00 - 0xb00
	.word 0x00000000
	.endr */

	.set __base,0xB00
	.rept 0xc00 - 0xb00
	FL_SECTION_ENTRY __base,3,0,0,0
	.set __base,__base+1
	.endr

	.set __base,0x200
	// 256MB for SDRAM with cacheable
	.rept 0xD00 - 0xC00
	FL_SECTION_ENTRY __base,3,0,1,1
	.set __base,__base+1
	.endr

	// access is not allowed.
	@.rept 0xD00 - 0xC80
	@.word 0x00000000
	@.endr

	.set __base,0xD00
	// 1:1 mapping for debugging with non-cacheable
	.rept 0x1000 - 0xD00
	FL_SECTION_ENTRY __base,3,0,0,0
	.set __base,__base+1
	.endr

#endif
