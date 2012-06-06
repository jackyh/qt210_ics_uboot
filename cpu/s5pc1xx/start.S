/*
 *  armboot - Startup Code for S3C6400/ARM1176 CPU-core
 *
 *  Copyright (c) 2007	Samsung Electronics
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * 2007-09-21 - Restructured codes by jsgood (jsgood.yang@samsung.com)
 * 2007-09-21 - Added moviNAND and OneNAND boot codes by jsgood (jsgood.yang@samsung.com)
 * Base codes by scsuh (sc.suh)
 */

#include <config.h>
#include <version.h>
#if defined(CONFIG_ENABLE_MMU)
#include <asm/proc/domain.h>
#endif
#include <regs.h>

#ifndef CONFIG_ENABLE_MMU
#ifndef CFG_PHY_UBOOT_BASE
#define CFG_PHY_UBOOT_BASE	CFG_UBOOT_BASE
#endif
#endif

/*
 * Information for iROM
 */

#if !defined(CONFIG_S5PC100_EVT1) && !defined(CONFIG_S5PC100_EVT2)
.word 0x2000
.word 0x0
.word 0x0
.word 0x0
#endif

/*
 *************************************************************************
 *
 * Jump vector table as in table 3.1 in [1]
 *
 *************************************************************************
 */

.globl _start
_start: b	reset
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction:
	.word undefined_instruction
_software_interrupt:
	.word software_interrupt
_prefetch_abort:
	.word prefetch_abort
_data_abort:
	.word data_abort
_not_used:
	.word not_used
_irq:
	.word irq
_fiq:
	.word fiq
_pad:
	.word 0x12345678 /* now 16*4=64 */
.global _end_vect
_end_vect:

	.balignl 16,0xdeadbeef
/*
 *************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * setup Memory and board specific bits prior to relocation.
 * relocate armboot to ram
 * setup stack
 *
 *************************************************************************
 */

_TEXT_BASE:
	.word	TEXT_BASE

/*
 * Below variable is very important because we use MMU in U-Boot.
 * Without it, we cannot run code correctly before MMU is ON.
 * by scsuh.
 */
_TEXT_PHY_BASE:
	.word	CFG_PHY_UBOOT_BASE

.globl _armboot_start
_armboot_start:
	.word _start

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

#if defined(CONFIG_USE_IRQ)
/* IRQ stack memory (calculated at run-time) */
.globl IRQ_STACK_START
IRQ_STACK_START:
	.word	0x0badc0de

/* IRQ stack memory (calculated at run-time) */
.globl FIQ_STACK_START
FIQ_STACK_START:
	.word 0x0badc0de
#endif

/*
 * the actual reset code
 */

reset:
	/*
	 * set the cpu to SVC32 mode and IRQ & FIQ disable
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

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

        /* Read booting information */
        ldr	r0, =PRO_ID_BASE
        ldr	r1, [r0,#OMR_OFFSET]
        bic	r2, r1, #0xfffffff9
        cmp	r2, #0x0
        moveq	r3, #BOOT_NAND
        cmp	r2, #0x2
        moveq	r3, #BOOT_ONENAND
        cmp      r2, #0x4
        moveq    r3, #BOOT_MMCSD	

	ldr	r0, =INF_REG_BASE
	str	r3, [r0, #INF_REG3_OFFSET]
	
	cmp	r3, #BOOT_NAND		/* 0x0 => boot device is nand */
	bne 	skip_nand_pin_mux
	ldr     r0, =0xe03002a0  	// GPK0CON
	ldr     r1, =0x22443322
	str     r1, [r0]
	ldr     r0, =0xe03002e0  	// GPK2CON
	ldr     r1, =0x33333333
	str     r1, [r0]  
skip_nand_pin_mux :
  
	/*
	 * Go setup Memory and board specific bits prior to relocation.
	 */

	ldr	sp, =0xd0036000 /* end of sram dedicated to u-boot */
	sub	sp, sp, #12	/* set stack */
	mov	fp, #0
	
	bl	lowlevel_init	/* go setup pll,mux,memory */

	ldr	r0, =INF_REG_BASE
	ldr	r1, [r0, #INF_REG3_OFFSET]
	cmp     r1, #BOOT_MMCSD
	bne     not_hclk_change

	ldr	r0, =0xff000fff		// If running PC value is in RAM. Not Change clk.
	bic	r1, pc, r0		/* r0 <- current base addr of code */
	ldr	r2, _TEXT_BASE		/* r1 <- original base addr in ram */
	bic	r2, r2, r0		/* r0 <- current base addr of code */
	cmp     r1, r2                  /* compare r0, r1                  */
	beq     not_hclk_change		/* r0 == r1 then skip flash copy   */

	ldr	r0, =ELFIN_CLOCK_POWER_BASE
	ldr   	r1, [r0, #CLK_DIV1_OFFSET]
	ldr	r2, =0xf000
	bic	r1, r1, r2
	ldr	r2, =0x5000
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV1_OFFSET]

not_hclk_change:
	/* when we already run in ram, we don't need to relocate U-Boot.
	 * and actually, memory controller must be configured before U-Boot
	 * is running in ram.
	 */
	ldr	r0, =0xff000fff
	bic	r1, pc, r0		/* r0 <- current base addr of code */
	ldr	r2, _TEXT_BASE		/* r1 <- original base addr in ram */
	bic	r2, r2, r0		/* r0 <- current base addr of code */
	cmp     r1, r2                  /* compare r0, r1                  */
	beq     after_copy		/* r0 == r1 then skip flash copy   */

	ldr	r0, =INF_REG_BASE
	ldr	r1, [r0, #INF_REG3_OFFSET]
	cmp	r1, #BOOT_NAND		/* 0x0 => boot device is nand */
	beq	nand_boot
	cmp	r1, #BOOT_ONENAND	/* 0x1 => boot device is onenand */
	beq	onenand_boot
	cmp     r1, #BOOT_MMCSD		/* 0x2 => boot device is SD/MMC */
	beq     mmcsd_boot

nand_boot:

	/* get ready to call C functions */
	ldr	sp, _TEXT_PHY_BASE	/* setup temp stack pointer */
	sub	sp, sp, #12
	mov	fp, #0			/* no previous frame, so fp=0 */
	bl	copy_from_nand
	b	after_copy

onenand_boot:

	ldr	r3, =ELFIN_ONENAND_BASE
	ldr	r4, =0xf6e2
	str	r4, [r3]

	ldr	r7, =CFG_PHY_UBOOT_BASE
	mov	r4, #0
	mov	r3, #0

page_increment:
	ldr	r2, =0xB3FFFF80
	add	r2, r2, r4, LSL #13
	mov	r8, #0x80
	add	r3, r3, #1
	add	r2, r2, r3, LSL #7

move_page:
	ldmia	r2, {r9-r12}
	stmia	r7!, {r9-r12}
	subs	r8, r8, #0x1
	bne	move_page

	ldr	r0, =0xe7100000
	mov	r1, #0x400
	str	r1, [r0, #0x50]

	/* 
		Changed BL2 copy size 
		256KB => 512KB
	*/

	cmp	r3, #255
	
	blt	page_increment

	add	r4, r4, #1
	mov	r3, #0xFFFFFFFF
	cmp	r4, #1
	blt	page_increment

	ldr	r0, =ELFIN_CLOCK_POWER_BASE
	ldr	r1, =0x00001111
	str	r1, [r0, #CLK_SRC0_OFFSET]

	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x11110000
	str	r1, [r0, #GPH1CON_OFFSET]

	ldr	r1, =0x5500
	str	r1, [r0, #GPH1PUD_OFFSET]

	ldr	r1, =0xf0
	str	r1, [r0, #GPH1DAT_OFFSET]

        ldr     r0, =ELFIN_GPIO_BASE
        ldr     r1, =0x3000
        str     r1, [r0, #0x834]

	b	after_copy

mmcsd_boot:

	ldr	r0, =APB_DMC_BASE			@APB_DMC_BASE 0xE6000000

	ldr	r1, =PRO_ID_BASE
	ldr	r2, [r1, #PRO_ID_OFFSET]
	bic	r2, #0xfffffdff
	mov	r2, r2, lsr #9
	cmp	r2, #0x1
	beq	jump_nand				@if pop then skip nand pin setting

	/* GPIO SET for NAND */
	ldr	r0, =ELFIN_GPIO_BASE
	
	ldr	r1, [r0, #GPK0CON_OFFSET]
	bic	r1, #0xff00
	ldr	r2, =0x3300
	orr	r1, r1 ,r2
	str	r1, [r0, #GPK0CON_OFFSET]
	
	ldr	r1, =0x33333333
	str	r1, [r0, #GPK2CON_OFFSET]

jump_nand:
        ldr     sp, _TEXT_PHY_BASE      
        sub     sp, sp, #12
        mov     fp, #0         
        bl      movi_bl2_copy
	
	ldr	r0, =ELFIN_CLOCK_POWER_BASE
	ldr   	r1, [r0, #CLK_DIV1_OFFSET]
	ldr	r2, =0xf000
	bic	r1, r1, r2
	ldr	r2, =0x1000
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV1_OFFSET]

	b       after_copy

after_copy:
#if defined(CONFIG_ENABLE_MMU)
enable_mmu:
	/* enable domain access */
	ldr	r5, =0x0000ffff
	mcr	p15, 0, r5, c3, c0, 0		@load domain access register

	/* Set the TTB register */
	ldr	r0, _mmu_table_base
	ldr	r1, =CFG_PHY_UBOOT_BASE
	ldr	r2, =0xfff00000
	bic	r0, r0, r2
	orr	r1, r0, r1
	mcr	p15, 0, r1, c2, c0, 0

	/* Enable the MMU */
mmu_on:
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #1
	mcr	p15, 0, r0, c1, c0, 0
	nop
	nop
	nop
	nop
#endif

skip_hw_init:
	/* Set up the stack						    */
stack_setup:
#if defined(CONFIG_MEMORY_UPPER_CODE)
	ldr	sp, =(CFG_UBOOT_BASE + CFG_UBOOT_SIZE - 0x1000)
#else
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CFG_MALLOC_LEN	/* malloc area                      */
	sub	r0, r0, #CFG_GBL_DATA_SIZE /* bdinfo                        */
#if defined(CONFIG_USE_IRQ)
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ)
#endif
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */

#endif

clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment        */
	ldr	r1, _bss_end		/* stop here                        */
	mov 	r2, #0x00000000		/* clear                            */

clbss_l:
	str	r2, [r0]		/* clear loop...                    */
	add	r0, r0, #4
	cmp	r0, r1
	ble	clbss_l
	
	ldr	pc, _start_armboot

_start_armboot:
	.word start_armboot

#if defined(CONFIG_ENABLE_MMU)
_mmu_table_base:
	.word mmu_table
#endif

/*
 * copy U-Boot to SDRAM and jump to ram (from NAND or OneNAND)
 * r0: size to be compared
 * Load 1'st 2blocks to RAM because U-boot's size is larger than 1block(128k) size
 */
	.globl copy_from_nand
copy_from_nand:
	push	{lr}

	mov	r9, #0x100		/* Compare about 8KB */
	bl	copy_uboot_to_ram
	tst 	r0, #0x0
	bne	copy_failed

/* FIXME: 0xd0034000 should be used for nand? */
#ifdef CONFIG_S5PC100_EVT1
	ldr	r0, =0xd0034000		/* Steeping Stone on S5PC100 */
#else
	ldr	r0, =0x34000
#endif

	ldr	r1, _TEXT_PHY_BASE	/* 0x27e00000 */
1:	ldr	r3, [r0], #4
	ldr	r4, [r1], #4
	teq	r3, r4
	bne	compare_failed	/* not matched */
	subs	r9, r9, #4
	bne	1b

	pop	{pc}

copy_failed:
	nop			/* copy from nand failed */
	b	copy_failed

compare_failed:
	nop			/* compare failed */
	b	compare_failed

/*
 * we assume that cache operation is done before. (eg. cleanup_before_linux())
 * actually, we don't need to do anything about cache if not use d-cache in U-Boot
 * So, in this function we clean only MMU. by scsuh
 *
 * void	theLastJump(void *kernel, int arch_num, uint boot_params);
 */
#if defined(CONFIG_ENABLE_MMU)
	.globl theLastJump
theLastJump:
	mov	r9, r0
	ldr	r3, =0xfff00000
	ldr	r4, _TEXT_PHY_BASE
	adr	r5, phy_last_jump
	bic	r5, r5, r3
	orr	r5, r5, r4
	mov	pc, r5
phy_last_jump:
	/*
	 * disable MMU stuff
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	/* clear bits 13, 9:8 (--V- --RS) */
	bic	r0, r0, #0x00000087	/* clear bits 7, 2:0 (B--- -CAM) */
	orr	r0, r0, #0x00000002	/* set bit 2 (A) Align */
	orr	r0, r0, #0x00001000	/* set bit 12 (I) I-Cache */
	mcr	p15, 0, r0, c1, c0, 0

	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	mov	r0, #0
	mov	pc, r9
#endif
/*
 *************************************************************************
 *
 * Interrupt handling
 *
 *************************************************************************
 */
@
@ IRQ stack frame.
@
#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52

#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#define MODE_SVC 0x13
#define I_BIT	 0x80

/*
 * use bad_save_user_regs for abort/prefetch/undef/swi ...
 * use irq_save_user_regs / irq_restore_user_regs for IRQ/FIQ handling
 */

	.macro	bad_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE		@ carve out a frame on current user stack
	stmia	sp, {r0 - r12}			@ Save user registers (now in svc mode) r0-r12

	ldr	r2, _armboot_start
	sub	r2, r2, #(CFG_MALLOC_LEN)
	sub	r2, r2, #(CFG_GBL_DATA_SIZE+8)	@ set base 2 words into abort stack
	ldmia	r2, {r2 - r3}			@ get values for "aborted" pc and cpsr (into parm regs)
	add	r0, sp, #S_FRAME_SIZE		@ grab pointer to old stack

	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r3}			@ save sp_SVC, lr_SVC, pc, cpsr
	mov	r0, sp				@ save current stack into r0 (param register)
	.endm

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			@ Calling r0-r12
	add	r8, sp, #S_PC			@ !!!! R8 NEEDS to be saved !!!! a reserved stack spot would be good.
	stmdb	r8, {sp, lr}^			@ Calling SP, LR
	str	lr, [r8, #0]			@ Save calling PC
	mrs	r6, spsr
	str	r6, [r8, #4]			@ Save CPSR
	str	r0, [r8, #8]			@ Save OLD_R0
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	subs	pc, lr, #4			@ return & move spsr_svc into cpsr
	.endm

	.macro get_bad_stack
	ldr	r13, _armboot_start		@ setup our mode stack (enter in banked mode)
	sub	r13, r13, #(CFG_MALLOC_LEN)	@ move past malloc pool
	sub	r13, r13, #(CFG_GBL_DATA_SIZE+8) @ move to reserved a couple spots for abort stack

	str	lr, [r13]			@ save caller lr in position 0 of saved stack
	mrs	lr, spsr			@ get the spsr
	str	lr, [r13, #4]			@ save spsr in position 1 of saved stack

	mov	r13, #MODE_SVC			@ prepare SVC-Mode
	@ msr	spsr_c, r13
	msr	spsr, r13			@ switch modes, make sure moves will execute
	mov	lr, pc				@ capture return pc
	movs	pc, lr				@ jump to next instruction & switch modes.
	.endm

	.macro get_bad_stack_swi
	sub	r13, r13, #4			@ space on current stack for scratch reg.
	str	r0, [r13]			@ save R0's value.
	ldr	r0, _armboot_start		@ get data regions start
	sub	r0, r0, #(CFG_MALLOC_LEN)	@ move past malloc pool
	sub	r0, r0, #(CFG_GBL_DATA_SIZE+8)	@ move past gbl and a couple spots for abort stack
	str	lr, [r0]			@ save caller lr in position 0 of saved stack
	mrs	r0, spsr			@ get the spsr
	str	lr, [r0, #4]			@ save spsr in position 1 of saved stack
	ldr	r0, [r13]			@ restore r0
	add	r13, r13, #4			@ pop stack entry
	.endm

	.macro get_irq_stack			@ setup IRQ stack
	ldr	sp, IRQ_STACK_START
	.endm

	.macro get_fiq_stack			@ setup FIQ stack
	ldr	sp, FIQ_STACK_START
	.endm

/*
 * exception handlers
 */
	.align	5
undefined_instruction:
	get_bad_stack
	bad_save_user_regs
	bl	do_undefined_instruction

	.align	5
software_interrupt:
	get_bad_stack_swi
	bad_save_user_regs
	bl	do_software_interrupt

	.align	5
prefetch_abort:
	get_bad_stack
	bad_save_user_regs
	bl	do_prefetch_abort

	.align	5
data_abort:
	get_bad_stack
	bad_save_user_regs
	bl	do_data_abort

	.align	5
not_used:
	get_bad_stack
	bad_save_user_regs
	bl	do_not_used

#if defined(CONFIG_USE_IRQ)

	.align	5
irq:
	get_irq_stack
	irq_save_user_regs
	bl	do_irq
	irq_restore_user_regs

	.align	5
fiq:
	get_fiq_stack
	/* someone ought to write a more effiction fiq_save_user_regs */
	irq_save_user_regs
	bl	do_fiq
	irq_restore_user_regs

#else

	.align	5
irq:
	get_bad_stack
	bad_save_user_regs
	bl	do_irq

	.align	5
fiq:
	get_bad_stack
	bad_save_user_regs
	bl	do_fiq

#endif
	.align 5
.global arm_cache_flush
arm_cache_flush:
       mcr     p15, 0, r1, c7, c5, 0           @ invalidate I cache
       mov     pc, lr                          @ back to caller

/*
 *     v7_flush_dcache_all()
 *
 *     Flush the whole D-cache.
 *
 *     Corrupted registers: r0-r5, r7, r9-r11
 *
 *     - mm    - mm_struct describing address space
 */
       .align 5
.global v7_flush_dcache_all
v7_flush_dcache_all:
       stmfd   r13!, {r0-r5, r7, r9-r12,r14}

       mov     r7, r0                          @ take a backup of device type
       cmp     r0, #0x3                        @ check if the device type is
                                               @ GP
       moveq r12, #0x1                         @ set up to invalide L2
smi:   .word 0x01600070                        @ Call SMI monitor (smieq)
       cmp     r7, #0x3                        @ compare again in case its
                                               @ lost
       beq     finished_inval                  @ if GP device, inval done
                                               @ above

       mrc     p15, 1, r0, c0, c0, 1           @ read clidr
       ands    r3, r0, #0x7000000              @ extract loc from clidr
       mov     r3, r3, lsr #23                 @ left align loc bit field
       beq     finished_inval                  @ if loc is 0, then no need to
                                               @ clean
       mov     r10, #0                         @ start clean at cache level 0
inval_loop1:
       add     r2, r10, r10, lsr #1            @ work out 3x current cache
                                               @ level
       mov     r1, r0, lsr r2                  @ extract cache type bits from
                                               @ clidr
       and     r1, r1, #7                      @ mask of the bits for current
                                               @ cache only
       cmp     r1, #2                          @ see what cache we have at
                                               @ this level
       blt     skip_inval                      @ skip if no cache, or just
                                               @ i-cache
       mcr     p15, 2, r10, c0, c0, 0          @ select current cache level
                                               @ in cssr
       mov     r2, #0                          @ operand for mcr SBZ
       mcr     p15, 0, r2, c7, c5, 4           @ flush prefetch buffer to
                                               @ sych the new cssr&csidr,
                                               @ with armv7 this is 'isb',
                                               @ but we compile with armv5
       mrc     p15, 1, r1, c0, c0, 0           @ read the new csidr
       and     r2, r1, #7                      @ extract the length of the
                                               @ cache lines
       add     r2, r2, #4                      @ add 4 (line length offset)
       ldr     r4, =0x3ff
       ands    r4, r4, r1, lsr #3              @ find maximum number on the
                                               @ way size
       clz     r5, r4                          @ find bit position of way
                                               @ size increment
       ldr     r7, =0x7fff
       ands    r7, r7, r1, lsr #13             @ extract max number of the
                                               @ index size
inval_loop2:
       mov     r9, r4                          @ create working copy of max
                                               @ way size
inval_loop3:
       orr     r11, r10, r9, lsl r5            @ factor way and cache number
                                               @ into r11
       orr     r11, r11, r7, lsl r2            @ factor index number into r11
       mcr     p15, 0, r11, c7, c6, 2          @ invalidate by set/way
       subs    r9, r9, #1                      @ decrement the way
       bge     inval_loop3
       subs    r7, r7, #1                      @ decrement the index
       bge     inval_loop2
skip_inval:
       add     r10, r10, #2                    @ increment cache number
       cmp     r3, r10
       bgt     inval_loop1
finished_inval:
       mov     r10, #0                         @ swith back to cache level 0
       mcr     p15, 2, r10, c0, c0, 0          @ select current cache level
                                               @ in cssr
       mcr     p15, 0, r10, c7, c5, 4          @ flush prefetch buffer,
                                               @ with armv7 this is 'isb',
                                               @ but we compile with armv5

       ldmfd   r13!, {r0-r5, r7, r9-r12,pc}


       .align  5


#if defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_CINTEGRATOR)
/* Use the IntegratorCP function from board/integratorcp/platform.S */
#elif defined(CONFIG_S3C64XX) || defined(CONFIG_S5PC1XX)
/* For future usage of S3C64XX*/
#else
	.align	5
.globl reset_cpu
reset_cpu:
	ldr	r1, rstctl	/* get addr for global reset reg */
	mov	r3, #0x2	/* full reset pll+mpu */
	str	r3, [r1]	/* force reset */
	mov	r0, r0
_loop_forever:
	b	_loop_forever
rstctl:
	.word	PM_RSTCTRL_WKUP

#endif

