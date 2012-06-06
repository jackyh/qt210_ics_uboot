/*
 * Copyright (c) 2009 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * Startup code for S5PC110
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

.include "s5pc110_reg.inc"

.equ	_STACK_BASEADDRESS,		0x33FF8000

@;/* Pre-defined constants */
.equ	USERMODE,			0x10
.equ	FIQMODE,			0x11
.equ	IRQMODE,			0x12
.equ	SVCMODE,			0x13
.equ	ABORTMODE,			0x17
.equ	UNDEFMODE,			0x1b
.equ	MODEMASK,			0x1f
.equ	NOINT,				0xc0

@;/* The location of stacks */
.equ	UserStack,		(_STACK_BASEADDRESS-0x3800) @;0x83ff4800 ~
.equ	SVCStack,		(_STACK_BASEADDRESS-0x2800) @;0x83ff5800 ~
.equ	UndefStack,		(_STACK_BASEADDRESS-0x2400) @;0x83ff5c00 ~
.equ	AbortStack,		(_STACK_BASEADDRESS-0x2000) @;0x83ff6000 ~
.equ	IRQStack,		(_STACK_BASEADDRESS-0x1000) @;0x83ff7000 ~
.equ	FIQStack,		(_STACK_BASEADDRESS-0x0)    @;0x83ff8000 ~

@;###############################################################################
.ifdef SYNC_16BURST_50M				@; DRAM clock is 100MHz
.equ		OneNAND_SYS_CONF1_Val,		0xC0E0
@;###############################################################################
.else	@; SYNC_16BURST_83M or ASYNC_READ	@; DRAM clock is 165MHz
.ifdef SYNC_16BURST_83M
.equ		OneNAND_SYS_CONF1_Val,		0xF006
.else
.equ		OneNAND_SYS_CONF1_Val,		0x40E0
.endif	@;SYNC_16BURST_83M
.endif	@;SYNC_16BURST_50M
@;###############################################################################

.globl Main

.section .text, "x"

.globl _start

_start:
	b	ResetHandler
@	b	HandlerUndef                    @;handler for Undefined mode
@	b	HandlerSWI                      @;handler for SWI interrupt
@	b	HandlerPabort                   @;handler for PAbort
@	b	HandlerDabort                   @;handler for DAbort
@	b	.                               @;reserved
@	b	HandlerIRQ                      @;handler for IRQ interrupt
@	b	HandlerFIQ                      @;handler for FIQ interrupt
@	b	.                               @;by digibuff FEB-23-2004 dummy branch

.ifdef zero_init
.globl _bss_start
_bss_start:
	.word bss_start

.globl _bss_end
_bss_end:
	.word bss_end

.globl _data_start
_data_start:
	.word data_start

.globl _rodata
_rodata:
	.word rodata
.endif

.equ		R1_I,	(1<<12)

ResetHandler:
	bl	reset
	bl	Main
	b	.


reset:
	/*
	 * set the cpu to SVC32 mode and IRQ & FIQ disable
	 */
	@;mrs	r0,cpsr
	@;bic	r0,r0,#0x1f
	@;orr	r0,r0,#0xd3
	@;msr	cpsr,r0
	msr	cpsr_c, #0xd3		@ I & F disable, Mode: 0x13 - SVC

/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */
	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
    */
cpu_init_crit:
@---------------------------------------------------------------
	/*
	 * Invalidate L1 I/D
	 */
	mov	r0, #0                  @ set up for MCR
	mcr	p15, 0, r0, c8, c7, 0   @ invalidate TLBs
	mcr	p15, 0, r0, c7, c5, 0   @ invalidate icache

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000     @ clear bits 13 (--V-)
	bic	r0, r0, #0x00000007     @ clear bits 2:0 (-CAM)
	orr	r0, r0, #0x00000002     @ set bit 1 (--A-) Align
	orr	r0, r0, #0x00000800     @ set bit 12 (Z---) BTB
	mcr 	p15, 0, r0, c1, c0, 0

	/*
	 * Go setup Memory and board specific bits prior to relocation.
	 */
	ldr	sp, =0xd0036000 /* end of sram dedicated to u-boot */
	sub	sp, sp, #12 /* set stack */
	mov	fp, #0

lowlevel_init:
	push	{lr}

	/* check reset status  */
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE+RST_STAT_OFFSET)
	ldr	r1, [r0]
	bic	r1, r1, #0xfff6ffff
	cmp	r1, #0x10000
	beq	wakeup_reset_pre
	cmp	r1, #0x80000
	beq	wakeup_reset_from_didle

	/* IO Retention release */
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE + OTHERS_OFFSET)	/* 0xE0100000 + 0xE000 */
	ldr	r1, [r0]
	ldr	r2, =IO_RET_REL
	orr	r1, r1, r2
	str	r1, [r0]

	/* Disable Watchdog */
	ldr	r0, =ELFIN_WATCHDOG_BASE	/* 0xE2700000 */
	mov	r1, #0
	str	r1, [r0]

	/* SRAM(2MB) init for SMDKC110 */
	/* GPJ1 SROM_ADDR_16to21 */
	ldr	r0, =ELFIN_GPIO_BASE
	
	ldr	r1, [r0, #GPJ1CON_OFFSET]
	bic	r1, r1, #0xFFFFFF
	ldr	r2, =0x444444
	orr	r1, r1, r2
	str	r1, [r0, #GPJ1CON_OFFSET]

	ldr	r1, [r0, #GPJ1PUD_OFFSET]
	ldr	r2, =0x3ff
	bic	r1, r1, r2
	str	r1, [r0, #GPJ1PUD_OFFSET]

	/* GPJ4 SROM_ADDR_16to21 */
	ldr	r1, [r0, #GPJ4CON_OFFSET]
	bic	r1, r1, #(0xf<<16)
	ldr	r2, =(0x4<<16)
	orr	r1, r1, r2
	str	r1, [r0, #GPJ4CON_OFFSET]

	ldr	r1, [r0, #GPJ4PUD_OFFSET]
	ldr	r2, =(0x3<<8)
	bic	r1, r1, r2
	str	r1, [r0, #GPJ4PUD_OFFSET]


	/* CS0 - 16bit sram, enable nBE, Byte base address */
	ldr	r0, =ELFIN_SROM_BASE	/* 0xE8000000 */
	mov	r1, #0x1
	str	r1, [r0]

	/* PS_HOLD pin(GPH0_0) set to high */
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE + PS_HOLD_CONTROL_OFFSET)
	ldr	r1, [r0]
	orr	r1, r1, #0x300	
	orr	r1, r1, #0x1	
	str	r1, [r0]

	/* init system clock */
	bl	system_clock_init

	bl	mem_ctrl_asm_init

1:
	/* for UART */
	bl	uart_asm_init

	bl	tzpc_init

	bl	onenandcon_init

	pop	{pc}

wakeup_reset_from_didle:
	/* Wait when APLL is locked */
	ldr	r0, =ELFIN_CLOCK_POWER_BASE
lockloop:
	ldr	r1, [r0, #APLL_CON0_OFFSET]
	and	r1, r1, #(1<<29)
	cmp	r1, #(1<<29)
	bne	lockloop
	beq	exit_wakeup

wakeup_reset_pre:
	mrc	p15, 0, r1, c1, c0, 1	@Read CP15 Auxiliary control register
	and	r1, r1, #0x80000000	@Check L2RD is disable or not
	cmp	r1, #0x80000000		
	bne	wakeup_reset		@if L2RD is not disable jump to wakeup_reset 
	
	bl	disable_l2cache
	bl	v7_flush_dcache_all
	bl	enable_l2cache

wakeup_reset:
	/* init system clock */
	bl	system_clock_init
	bl	mem_ctrl_asm_init
	bl	tzpc_init
	bl 	onenandcon_init

exit_wakeup:
	/*Load return address and jump to kernel*/
	ldr	r0, =(INF_REG_BASE+INF_REG0_OFFSET)
	ldr	r1, [r0]	/* r1 = physical address of s5pc110_cpu_resume function*/

	mov	pc, r1		/*Jump to kernel */
	nop
	nop

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
	str	r1, [r0, #APLL_CON0_OFFSET]
	ldr	r1, =0x0
	str	r1, [r0, #MPLL_CON_OFFSET]

	ldr	r1, =0x0
	str	r1, [r0, #MPLL_CON_OFFSET]

	ldr   	r1, [r0, #CLK_DIV0_OFFSET]
	ldr	r2, =CLK_DIV0_MASK
	bic	r1, r1, r2

	ldr	r2, =CLK_DIV0_VAL
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV0_OFFSET]

	ldr	r1, =APLL_VAL
	str	r1, [r0, #APLL_CON0_OFFSET]

	ldr	r1, =MPLL_VAL
	str	r1, [r0, #MPLL_CON_OFFSET]

	ldr	r1, =VPLL_VAL
	str	r1, [r0, #VPLL_CON_OFFSET]
.ifdef S5PC110_EVT1
	ldr	r1, =AFC_ON
	str	r1, [r0, #APLL_CON1_OFFSET]
.endif

	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

	/* Set Mux to PLL (Bus clock) */

	/* CLK_SRC0 PLLsel -> APLLout(MSYS), MPLLout(DSYS,PSYS), EPLLout, VPLLout */
	ldr	r1, [r0, #CLK_SRC0_OFFSET]
	ldr	r2, =0x10001111
	orr	r1, r1, r2
	str	r1, [r0, #CLK_SRC0_OFFSET]

.ifdef CONFIG_SMDKC110AC
	/* CLK_SRC6[25:24] -> OneDRAM clock sel = MPLL */
	ldr	r1, [r0, #CLK_SRC6_OFFSET]
	bic	r1, r1, #(0x3<<24)
	orr	r1, r1, #0x01000000
	str	r1, [r0, #CLK_SRC6_OFFSET]

	/* CLK_DIV6[31:28] -> 4=1/5, 3=1/4(166MHZ@667MHz), 2=1/3 */
	ldr	r1, [r0, #CLK_DIV6_OFFSET]
	bic	r1, r1, #(0xF<<28)
	orr	r1, r1, #0x30000000
	str	r1, [r0, #CLK_DIV6_OFFSET]
.endif
.ifdef CONFIG_SMDKC110B
	/* CLK_SRC6[25:24] -> OneDRAM clock sel = 00:SCLKA2M, 01:SCLKMPLL */
	ldr	r1, [r0, #CLK_SRC6_OFFSET]
	bic	r1, r1, #(0x3<<24)
	orr	r1, r1, #0x01000000
	str	r1, [r0, #CLK_SRC6_OFFSET]

	/* CLK_DIV6[31:28] -> 4=1/5, 3=1/4(166MHZ@667MHz), 2=1/3 */
	ldr	r1, [r0, #CLK_DIV6_OFFSET]
	bic	r1, r1, #(0xF<<28)
	orr	r1, r1, #0x30000000
	str	r1, [r0, #CLK_DIV6_OFFSET]
.endif
.ifdef CONFIG_SMDKC110D
	/* CLK_SRC6[25:24] -> OneDRAM clock sel = 00:SCLKA2M, 01:SCLKMPLL */
	ldr	r1, [r0, #CLK_SRC6_OFFSET]
	bic	r1, r1, #(0x3<<24)
	orr	r1, r1, #0x01000000
	str	r1, [r0, #CLK_SRC6_OFFSET]

	/* CLK_DIV6[31:28] -> 4=1/5, 3=1/4(166MHZ@667MHz), 2=1/3 */
	ldr	r1, [r0, #CLK_DIV6_OFFSET]
	bic	r1, r1, #(0xF<<28)
	orr	r1, r1, #0x30000000
	str	r1, [r0, #CLK_DIV6_OFFSET]
.endif
.ifdef CONFIG_SMDKC110H
	/* CLK_SRC6[25:24] -> OneDRAM clock sel = 00:SCLKA2M, 01:SCLKMPLL */
	ldr	r1, [r0, #CLK_SRC6_OFFSET]
	bic	r1, r1, #(0x3<<24)
	orr	r1, r1, #0x00000000
	str	r1, [r0, #CLK_SRC6_OFFSET]

	/* CLK_DIV6[31:28] -> 4=1/5, 3=1/4(166MHZ@667MHz), 2=1/3 */
	ldr	r1, [r0, #CLK_DIV6_OFFSET]
	bic	r1, r1, #(0xF<<28)
	orr	r1, r1, #0x00000000
	str	r1, [r0, #CLK_DIV6_OFFSET]
.endif

	/* CLK OUT Setting */
	/* DIVVAL[23:20], CLKSEL[16:12] */
	ldr	r1, [r0, #CLK_OUT_OFFSET]
	ldr	r2, =0x00909000
	orr	r1, r1, r2
	str	r1, [r0, #CLK_OUT_OFFSET]

	mov	pc, lr

/*
 * uart_asm_init: Initialize UART in asm mode, 115200bps fixed.
 * void uart_asm_init(void)
 */
uart_asm_init:
	/* set GPIO(GPA) to enable UART */
	@ GPIO setting for UART
	ldr	r0, =ELFIN_GPIO_BASE	@0xE0200000
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

	ldr	r1, =0x4C4C4C4C
	str	r1, [r0, #UTXH_OFFSET]		@'L'

	mov	pc, lr

/*
 * Setting TZPC[TrustZone Protection Controller]
 */

tzpc_init:

	ldr	r0, =ELFIN_TZPC0_BASE
	mov	r1, #0x0
	str	r1, [r0]
	mov	r1, #0xff
	str	r1, [r0, #TZPC_DECPROT0SET_OFFSET]
	str	r1, [r0, #TZPC_DECPROT1SET_OFFSET]
	str	r1, [r0, #TZPC_DECPROT2SET_OFFSET]

	ldr	r0, =ELFIN_TZPC1_BASE
	str	r1, [r0, #TZPC_DECPROT0SET_OFFSET]
	str	r1, [r0, #TZPC_DECPROT1SET_OFFSET]
	str	r1, [r0, #TZPC_DECPROT2SET_OFFSET]

	ldr	r0, =ELFIN_TZPC2_BASE
	str	r1, [r0, #TZPC_DECPROT0SET_OFFSET]
	str	r1, [r0, #TZPC_DECPROT1SET_OFFSET]
	str	r1, [r0, #TZPC_DECPROT2SET_OFFSET]
	str	r1, [r0, #TZPC_DECPROT3SET_OFFSET]

	ldr	r0, =ELFIN_TZPC3_BASE
	str	r1, [r0, #TZPC_DECPROT0SET_OFFSET]
	str	r1, [r0, #TZPC_DECPROT1SET_OFFSET]
	str	r1, [r0, #TZPC_DECPROT2SET_OFFSET]

	mov	pc, lr

mem_ctrl_asm_init:

	ldr	r0, =ASYNC_MSYS_DMC0_BASE
	ldr	r1, =0x0
	str	r1, [r0, #0x0]

	ldr	r1, =0x0
	str	r1, [r0, #0xC]

.ifdef CONFIG_SMDKV210
	/* DMC0 Drive Strength (Setting 2X) */
	ldr	r0, =ELFIN_GPIO_BASE

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_0DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_1DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_2DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_3DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_4DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_5DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_6DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_7DRV_SR_OFFSET]

	ldr	r1, =0x00002AAA
	str	r1, [r0, #MP1_8DRV_SR_OFFSET]


	/* DMC1 Drive Strength (Setting 2X) */

	ldr	r0, =ELFIN_GPIO_BASE

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP2_0DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP2_1DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP2_2DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP2_3DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP2_4DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP2_5DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP2_6DRV_SR_OFFSET]

	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP2_7DRV_SR_OFFSET]

	ldr	r1, =0x00002AAA
	str	r1, [r0, #MP2_8DRV_SR_OFFSET]

	/* DMC0 initialization at single Type*/
	ldr	r0, =APB_DMC_0_BASE

	ldr	r1, =0x00101000				@PhyControl0 DLL parameter setting, manual 0x00101000
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x00000086				@PhyControl1 DLL parameter setting, LPDDR/LPDDR2 Case
	str	r1, [r0, #DMC_PHYCONTROL1]

	ldr	r1, =0x00101002				@PhyControl0 DLL on
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x00101003				@PhyControl0 DLL start
	str	r1, [r0, #DMC_PHYCONTROL0]

find_lock_val:
	ldr	r1, [r0, #DMC_PHYSTATUS]		@Load Phystatus register value
	and	r2, r1, #0x7
	cmp	r2, #0x7				@Loop until DLL is locked
	bne	find_lock_val

	and	r1, #0x3fc0
	mov	r2, r1, LSL #18
	orr	r2, r2, #0x100000
	orr	r2 ,r2, #0x1000

	orr	r1, r2, #0x3				@Force Value locking
	str	r1, [r0, #DMC_PHYCONTROL0]

.if 0	/* Memory margin test 10.01.05 */
	orr	r1, r2, #0x1				@DLL off
	str	r1, [r0, #DMC_PHYCONTROL0]
.endif
	/* setting DDR2 */
	ldr	r1, =0x0FFF2010				@ConControl auto refresh off
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =0x00212400				@MemControl BL=4, 2 chip, DDR2 type, dynamic self refresh, force precharge, dynamic power down off
	str	r1, [r0, #DMC_MEMCONTROL]

	ldr	r1, =DMC0_MEMCONFIG_0			@MemConfig0 256MB config, 8 banks,Mapping Method[12:15]0:linear, 1:linterleaved, 2:Mixed
	str	r1, [r0, #DMC_MEMCONFIG0]

	ldr	r1, =DMC0_MEMCONFIG_1			@MemConfig1
	str	r1, [r0, #DMC_MEMCONFIG1]

	ldr	r1, =0xFF000000				@PrechConfig
	str	r1, [r0, #DMC_PRECHCONFIG]

	ldr	r1, =DMC0_TIMINGA_REF			@TimingAref	7.8us*133MHz=1038(0x40E), 100MHz=780(0x30C), 20MHz=156(0x9C), 10MHz=78(0x4E)
	str	r1, [r0, #DMC_TIMINGAREF]

	ldr	r1, =DMC0_TIMING_ROW			@TimingRow	for @200MHz
	str	r1, [r0, #DMC_TIMINGROW]

	ldr	r1, =DMC0_TIMING_DATA			@TimingData	CL=3
	str	r1, [r0, #DMC_TIMINGDATA]

	ldr	r1, =DMC0_TIMING_PWR			@TimingPower
	str	r1, [r0, #DMC_TIMINGPOWER]

	ldr	r1, =0x07000000				@DirectCmd	chip0 Deselect
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01000000				@DirectCmd	chip0 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00020000				@DirectCmd	chip0 EMRS2
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00030000				@DirectCmd	chip0 EMRS3
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00010400				@DirectCmd	chip0 EMRS1 (MEM DLL on, DQS# disable)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00000542				@DirectCmd	chip0 MRS (MEM DLL reset) CL=4, BL=4
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01000000				@DirectCmd	chip0 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05000000				@DirectCmd	chip0 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05000000				@DirectCmd	chip0 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00000442				@DirectCmd	chip0 MRS (MEM DLL unreset)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00010780				@DirectCmd	chip0 EMRS1 (OCD default)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00010400				@DirectCmd	chip0 EMRS1 (OCD exit)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x07100000				@DirectCmd	chip1 Deselect
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01100000				@DirectCmd	chip1 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00120000				@DirectCmd	chip1 EMRS2
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00130000				@DirectCmd	chip1 EMRS3
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00110400				@DirectCmd	chip1 EMRS1 (MEM DLL on, DQS# disable)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00100542				@DirectCmd	chip1 MRS (MEM DLL reset) CL=4, BL=4
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01100000				@DirectCmd	chip1 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05100000				@DirectCmd	chip1 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05100000				@DirectCmd	chip1 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00100442				@DirectCmd	chip1 MRS (MEM DLL unreset)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00110780				@DirectCmd	chip1 EMRS1 (OCD default)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00110400				@DirectCmd	chip1 EMRS1 (OCD exit)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x0FF02030				@ConControl	auto refresh on
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =0xFFFF00FF				@PwrdnConfig
	str	r1, [r0, #DMC_PWRDNCONFIG]

	ldr	r1, =0x00202400				@MemControl	BL=4, 2 chip, DDR2 type, dynamic self refresh, force precharge, dynamic power down off
	str	r1, [r0, #DMC_MEMCONTROL]

	/* DMC1 initialization */
	ldr	r0, =APB_DMC_1_BASE

	ldr	r1, =0x00101000				@Phycontrol0 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x00000086				@Phycontrol1 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL1]

	ldr	r1, =0x00101002				@PhyControl0 DLL on
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x00101003				@PhyControl0 DLL start
	str	r1, [r0, #DMC_PHYCONTROL0]
find_lock_val1:
	ldr	r1, [r0, #DMC_PHYSTATUS]		@Load Phystatus register value
	and	r2, r1, #0x7
	cmp	r2, #0x7				@Loop until DLL is locked
	bne	find_lock_val1

	and	r1, #0x3fc0 
	mov	r2, r1, LSL #18
	orr	r2, r2, #0x100000
	orr	r2, r2, #0x1000

	orr	r1, r2, #0x3				@Force Value locking
	str	r1, [r0, #DMC_PHYCONTROL0]

.if 0	/* Memory margin test 10.01.05 */
	orr	r1, r2, #0x1				@DLL off
	str	r1, [r0, #DMC_PHYCONTROL0]
.endif

	/* settinf fot DDR2 */
	ldr	r0, =APB_DMC_1_BASE

	ldr	r1, =0x0FFF2010				@auto refresh off
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =DMC1_MEMCONTROL			@MemControl	BL=4, 2 chip, DDR2 type, dynamic self refresh, force precharge, dynamic power down off
	str	r1, [r0, #DMC_MEMCONTROL]

	ldr	r1, =DMC1_MEMCONFIG_0			@MemConfig0	512MB config, 8 banks,Mapping Method[12:15]0:linear, 1:linterleaved, 2:Mixed
	str	r1, [r0, #DMC_MEMCONFIG0]

	ldr	r1, =DMC1_MEMCONFIG_1			@MemConfig1
	str	r1, [r0, #DMC_MEMCONFIG1]

	ldr	r1, =0xFF000000
	str	r1, [r0, #DMC_PRECHCONFIG]

	ldr	r1, =DMC1_TIMINGA_REF			@TimingAref	7.8us*133MHz=1038(0x40E), 100MHz=780(0x30C), 20MHz=156(0x9C), 10MHz=78(0x4
	str	r1, [r0, #DMC_TIMINGAREF]

	ldr	r1, =DMC1_TIMING_ROW			@TimingRow	for @200MHz
	str	r1, [r0, #DMC_TIMINGROW]

	ldr	r1, =DMC1_TIMING_DATA			@TimingData	CL=3
	str	r1, [r0, #DMC_TIMINGDATA]

	ldr	r1, =DMC1_TIMING_PWR			@TimingPower
	str	r1, [r0, #DMC_TIMINGPOWER]


	ldr	r1, =0x07000000				@DirectCmd	chip0 Deselect
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01000000				@DirectCmd	chip0 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00020000				@DirectCmd	chip0 EMRS2
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00030000				@DirectCmd	chip0 EMRS3
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00010400				@DirectCmd	chip0 EMRS1 (MEM DLL on, DQS# disable)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00000542				@DirectCmd	chip0 MRS (MEM DLL reset) CL=4, BL=4
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01000000				@DirectCmd	chip0 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05000000				@DirectCmd	chip0 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05000000				@DirectCmd	chip0 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00000442				@DirectCmd	chip0 MRS (MEM DLL unreset)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00010780				@DirectCmd	chip0 EMRS1 (OCD default)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00010400				@DirectCmd	chip0 EMRS1 (OCD exit)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x07100000				@DirectCmd	chip1 Deselect
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01100000				@DirectCmd	chip1 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00120000				@DirectCmd	chip1 EMRS2
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00130000				@DirectCmd	chip1 EMRS3
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00110440				@DirectCmd	chip1 EMRS1 (MEM DLL on, DQS# disable)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00100542				@DirectCmd	chip1 MRS (MEM DLL reset) CL=4, BL=4
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01100000				@DirectCmd	chip1 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05100000				@DirectCmd	chip1 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05100000				@DirectCmd	chip1 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00100442				@DirectCmd	chip1 MRS (MEM DLL unreset)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00110780				@DirectCmd	chip1 EMRS1 (OCD default)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00110400				@DirectCmd	chip1 EMRS1 (OCD exit)
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x0FF02030				@ConControl	auto refresh on
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =0xFFFF00FF				@PwrdnConfig
	str	r1, [r0, #DMC_PWRDNCONFIG]

	ldr	r1, =DMC1_MEMCONTROL			@MemControl	BL=4, 2 chip, DDR2 type, dynamic self refresh, force precharge, dynamic power down off
	str	r1, [r0, #DMC_MEMCONTROL]
.else
	/* DMC0 initialization */
	ldr	r0, =APB_DMC_0_BASE

	ldr	r1, =0x00101000				@Phycontrol0 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x00000084				@Phycontrol1 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL1]

	ldr	r1, =0x00101002				@Phycontrol2 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x00101003				@Dll on
	str	r1, [r0, #DMC_PHYCONTROL0]

	mov	r2, #0x4000	/* Set for wait1 */
wait1:	/* To be impleneted*/
	subs	r2, r2, #0x1
	cmp	r2, #0x0
	bne	wait1

	ldr	r1, =0x82101003				@Force Value locking
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x82101009				@Dll off
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x0fff1010				@auto refresh off
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =0x00212100
	str	r1, [r0, #DMC_MEMCONTROL]

	ldr	r1, =DMC0_MEMCONFIG_0
	str	r1, [r0, #DMC_MEMCONFIG0]

	ldr	r1, =DMC0_MEMCONFIG_1
	str	r1, [r0, #DMC_MEMCONFIG1]

	ldr	r1, =0xff000000
	str	r1, [r0, #DMC_PRECHCONFIG]

	ldr	r1, =DMC0_TIMINGA_REF
	str	r1, [r0, #DMC_TIMINGAREF]

	ldr	r1, =DMC0_TIMING_ROW			@TimingRow	@133MHz
	str	r1, [r0, #DMC_TIMINGROW]

	ldr	r1, =DMC0_TIMING_DATA
	str	r1, [r0, #DMC_TIMINGDATA]

	ldr	r1, =DMC0_TIMING_PWR			@Timing Power
	str	r1, [r0, #DMC_TIMINGPOWER]

	ldr	r1, =0x07000000				@chip0 Deselect
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01000000				@chip0 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05000000				@chip0 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05000000				@chip0 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00000032				@chip0 MRS
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x07100000				@chip1 Deselect
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01100000				@chip1 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05100000				@chip1 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05100000				@chip1 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00100032				@chip1 MRS
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x0FFF10B0			@ConControl auto refresh on
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =0xFFFF00FF				@PwrdnConfig
	str	r1, [r0, #DMC_PWRDNCONFIG]

.ifdef CONFIG_SMDKC110AC
	ldr	r1, =0x00212113				@MemControl
	str	r1, [r0, #DMC_MEMCONTROL]
.endif
.ifdef CONFIG_SMDKC110B
	ldr	r1, =0x00212113				@MemControl
	str	r1, [r0, #DMC_MEMCONTROL]
.endif
.ifdef CONFIG_SMDKC110D
	ldr	r1, =0x00212113				@MemControl
	str	r1, [r0, #DMC_MEMCONTROL]
.endif
.ifdef CONFIG_SMDKC110H
	ldr	r1, =0x00212113				@MemControl
	str	r1, [r0, #DMC_MEMCONTROL]
.endif

	/* DMC1 initialization */
	ldr	r0, =APB_DMC_1_BASE

	ldr	r1, =0x00101000				@Phycontrol0 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x00000084				@Phycontrol1 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL1]

	ldr	r1, =0x00101002				@Phycontrol2 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x00101003				@Dll on
	str	r1, [r0, #DMC_PHYCONTROL0]

	mov	r2, #0x4000
wait2:	/* To be impleneted*/
	subs	r2, r2, #0x1
	cmp	r2, #0x0
	bne	wait2

	ldr	r1, =0x71101003				@Force Value locking
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x71101009				@Dll off
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r0, =APB_DMC_1_BASE

	ldr	r1, =0x0FFF1010				@auto refresh off
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =0x00212100
	str	r1, [r0, #DMC_MEMCONTROL]

	ldr	r1, =DMC1_MEMCONFIG_0
	str	r1, [r0, #DMC_MEMCONFIG0]

	ldr	r1, =DMC1_MEMCONFIG_1
	str	r1, [r0, #DMC_MEMCONFIG1]

	ldr	r1, =0xff000000
	str	r1, [r0, #DMC_PRECHCONFIG]

	ldr	r1, =DMC1_TIMINGA_REF
	str	r1, [r0, #DMC_TIMINGAREF]

	ldr	r1, =DMC1_TIMING_ROW			@TimingRow	@133MHz
	str	r1, [r0, #DMC_TIMINGROW]

	ldr	r1, =DMC1_TIMING_DATA
	str	r1, [r0, #DMC_TIMINGDATA]

	ldr	r1, =DMC1_TIMING_PWR			@Timing Power
	str	r1, [r0, #DMC_TIMINGPOWER]

	ldr	r1, =0x07000000				@chip0 Deselect
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01000000				@chip0 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05000000				@chip0 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05000000				@chip0 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00000032				@chip0 MRS
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x07100000				@chip1 Deselect
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x01100000				@chip1 PALL
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05100000				@chip1 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x05100000				@chip1 REFA
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x00100032				@chip1 MRS
	str	r1, [r0, #DMC_DIRECTCMD]

	ldr	r1, =0x0FFF10B0				@ConControl auto refresh on
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =0xFFFF00FF				@PwrdnConfig
	str	r1, [r0, #DMC_PWRDNCONFIG]

	ldr	r1, =0x00212113				@MemControl
	str	r1, [r0, #DMC_MEMCONTROL]
.endif

	mov	pc, lr

onenandcon_init:
	@; GPIO setting for OneNAND
	ldr	r0, =ELFIN_GPIO_BASE	@0xE0200000
	ldr	r1, [r0, #MP01CON_OFFSET]
	orr	r1, r1, #0x00550000
	str	r1, [r0, #MP01CON_OFFSET]

	ldr	r1, [r0, #MP03CON_OFFSET]
	orr	r1, r1, #0x0550
	orr	r1, r1, #0x00550000
	str	r1, [r0, #MP03CON_OFFSET]

	ldr	r1, =0xFFFF
	str	r1, [r0, #MP01DRV_SR_OFFSET]
	str	r1, [r0, #MP03DRV_SR_OFFSET]
	str	r1, [r0, #MP06DRV_SR_OFFSET]
	str	r1, [r0, #MP07DRV_SR_OFFSET]

wait_orwb:
	@; Read ONENAND_IF_STATUS
	ldr	r0, =ELFIN_ONENANDCON_BASE	@; 0xB0600000
	ldr	r1, [r0, #ONENAND_IF_STATUS_OFFSET]
	bic	r1, r1, #0xFFFFFFFE
	cmp	r1, #0x0

	@; ORWB != 0x0
	bne	wait_orwb

	@; write new configuration to onenand system configuration1 register
	ldr	r1, =OneNAND_SYS_CONF1_Val
	ldr	r2, =(OneNAND_CS0_Base+0x1E442)	@; 0xB0000000+0x1E442(REG_SYS_CONF1)
	strh	r1, [r2]

	@; read one dummy halfword
	ldrh	r1, [r2]
	ldrh	r1, [r2]

	@; write new configuration to ONENAND_IF_CTRL
	ldr	r0, =ELFIN_ONENANDCON_BASE	@; 0xB0600000
	ldr	r1, =ONENAND_IF_CTRL_REG_VAL
	str	r1, [r0, #ONENAND_IF_CTRL_OFFSET]

	mov	pc, lr

/*
 *     v7_flush_dcache_all()
 *
 *     Flush the whole D-cache.
 *
 *     Corrupted registers: r0-r5, r7, r9-r11
 *
 *     - mm    - mm_struct describing address space
 */
v7_flush_dcache_all:
	ldr	r0, =0xffffffff
	mrc	p15, 1, r0, c0, c0, 1 		@ Read CLIDR
	ands	r3, r0, #0x7000000
	mov	r3, r3, LSR #23       		@ Cache level value (naturally aligned)
	beq	Finished
	mov	r10, #0
Loop1:         
	add	r2, r10, r10, LSR #1  		@ Work out 3xcachelevel
	mov	r1, r0, LSR r2        		@ bottom 3 bits are the Ctype for this level
	and	r1, r1, #7            		@ get those 3 bits alone
	cmp	r1, #2
	blt	Skip                   		@ no cache or only instruction cache at this level
	mcr	p15, 2, r10, c0, c0, 0 		@ write the Cache Size selection register
	mov	r1, #0
	mcr	p15, 0, r1, c7, c5, 4 		@ PrefetchFlush to sync the change to the CacheSizeID reg
	mrc	p15, 1, r1, c0, c0, 0 		@ reads current Cache Size ID register
	and	r2, r1, #0x7           		@ extract the line length field
	add	r2, r2, #4            		@ add 4 for the line length offset (log2 16 bytes)
	ldr	r4, =0x3FF
	ands	r4, r4, r1, LSR #3   		@ R4 is the max number on the way size (right aligned)
	clz	r5, r4                		@ R5 is the bit position of the way size increment
	ldr	r7, =0x00007FFF
	ands	r7, r7, r1, LSR #13  		@ R7 is the max number of the index size (right aligned)
Loop2:         
	mov	r9, r4                      	@ R9 working copy of the max way size (right aligned)
Loop3:         
	orr	r11, r10, r9, LSL r5        	@ factor in the way number and cache number into R11
	orr	r11, r11, r7, LSL r2        	@ factor in the index number
	mcr	p15, 0, r11, c7, c6, 2 		@ invalidate by set/way
	subs	r9, r9, #1                 	@ decrement the way number
	bge	Loop3
	subs	r7, r7, #1                 	@ decrement the index
	bge	Loop2
Skip:          
	add	r10, r10, #2                	@ increment the cache number
	cmp	r3, r10
	bgt	Loop1
Finished:
	mov	pc, lr
	
disable_l2cache:
	mrc	p15, 0, r0, c1, c0, 1
	bic	r0, r0, #(1<<1)
	mcr	p15, 0, r0, c1, c0, 1
	mov	pc, lr

enable_l2cache:
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, r0, #(1<<1)
	mcr	p15, 0, r0, c1, c0, 1
	mov	pc, lr

.if 0
set_l2cache_auxctrl:
	mov	r0, #0x0
	mcr	p15, 1, r0, c9, c0, 2
	mov	pc, lr

set_l2cache_auxctrl_cycle:
	mrc p15, 1, r0, c9, c0, 2
	mov r1, #(0x1<<29)
	mov r2, #(0x1)
	bic r0, r0, #0x7
	orr r0, r0, r1
	orr r0, r0, r2
	mcr p15, 1, r0, c9, c0, 2
	mov     pc,lr

CoInvalidateDCacheIndex:
	;/* r0 = index */
	mcr	p15, 0, r0, c7, c6, 2
	mov	pc,lr
.endif

@;The LR register wont be valid if the current mode is not SVC mode.
HandlerFIQ:
	b	.
HandlerIRQ:
	b	.
HandlerUndef:
	b	.
HandlerSWI:
	b	.
HandlerDabort:
	b	.
HandlerPabort:
	b	.

.end
