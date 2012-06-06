#include <config.h>
#include <s5p6442.h>
 
	.globl mem_ctrl_asm_init
mem_ctrl_asm_init:
	/* SROMC Bus Control for BANK0 */
	ldr	r0, =ELFIN_SROM_BASE	/* 0xE7000000 */
	mov	r1, #0xb8
	str	r1, [r0]

	/* when BUS = 12MHz */
	mov	r1, #0x0
	str	r1, [r0, #0x4]

	/* GPJ0 SROM_ADDR_16~20 set 20 set */
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x3333333
	str	r1, [r0, #GPJ0CON_OFFSET]
	
	/* SDRAM DMC0 Initialization */
	/* dll on at133MHz */
	ldr	r0, =APB_DMC_0_BASE			@APB_DMC_BASE 0xE6000000

	ldr     r1, =0x00141408                         @Phycontrol0 DLL parameter setting
	str     r1, [r0, #DMC_PHYCONTROL0]

#ifdef CONFIG_SMDK6442_EVT1
	ldr     r1, =0x00000044                         @Phycontrol1 DLL parameter setting
#else
	ldr     r1, =0x00008044                         @Phycontrol1 DLL parameter setting
#endif
	str     r1, [r0, #DMC_PHYCONTROL1]

	ldr     r1, =0x0014140A                         @Dll on
	str     r1, [r0, #DMC_PHYCONTROL0]

	ldr     r1, =0x0014140B                         @dll start
	str     r1, [r0, #DMC_PHYCONTROL0]

	mov     r2, #0x4000
wait1:
	subs    r2, r2, #0x1
	cmp     r2, #0x0
	bne     wait1
#ifdef CONFIG_MCP_C
	/* Setting LPDDR */
 	ldr	r1, =0x0F0010D0				@ConControl	auto refresh off, rd_fetch = 1(could be 0 for performance), out_of_order = 1
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =0x00212101				@MemControl	BL=4, 1 chip, LPDDR, dynamic self refresh off, force precharge off, dynamic power down off, clock always running
	str	r1, [r0, #DMC_MEMCONTROL]

#ifdef CONFIG_SMDK6442_EVT1
	ldr	r1, =0x40F00222				@MemConfig0	1Gbit config, 4banks, col=10, row=13, bank=4
#else
	ldr	r1, =0x40F80222				@MemConfig0	1Gbit config, 4banks, col=10, row=13, bank=4
#endif
	str	r1, [r0, #DMC_MEMCONFIG0]

	ldr     r1, =0x20F00222
	str     r1, [r0, #DMC_MEMCONFIG1]

	ldr     r1, =0xf0000000
	str     r1, [r0, #DMC_PRECHCONFIG]

	ldr     r1, =0x0000040E
	str     r1, [r0, #DMC_TIMINGAREF]

	ldr     r1, =0x0C233286                         @TimingRow      @133MHz
	str     r1, [r0, #DMC_TIMINGROW]

	ldr     r1, =0x12130005
	str     r1, [r0, #DMC_TIMINGDATA]

	ldr     r1, =0x0E120222                         @Timing Power
	str     r1, [r0, #DMC_TIMINGPOWER]

	ldr     r1, =0x07000000                         @chip0 Deselect
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x01000000                         @chip0 PALL
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x05000000                         @chip0 REFA
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x05000000                         @chip0 REFA
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x00000032                         @chip0 MRS
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x07100000                         @chip0 Deselect
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x01100000                         @chip0 PALL
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x05100000                         @chip0 REFA
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x05100000                         @chip0 REFA
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x00100032                         @chip0 MRS
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x0F0010F0                         @ConControl auto refresh on
	str     r1, [r0, #DMC_CONCONTROL]

	ldr     r1, =0x00100002                         @PwrdnConfig
	str     r1, [r0, #DMC_PWRDNCONFIG]

	ldr     r1, =0x00212101                         @MemControl
	str     r1, [r0, #DMC_MEMCONTROL]
#else
	/* Setting LPDDR */
 	ldr	r1, =0x0F001090				@ConControl	auto refresh off, rd_fetch = 1(could be 0 for performance), out_of_order = 1
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =0x00202100				@MemControl	BL=4, 1 chip, LPDDR, dynamic self refresh off, force precharge off, dynamic power down off, clock always running
	str	r1, [r0, #DMC_MEMCONTROL]

	ldr	r1, =0x20F00312				@MemConfig0	1Gbit config, 4banks, col=10, row=13, bank=4
	str	r1, [r0, #DMC_MEMCONFIG0]

	ldr     r1, =0x30F00222
	str     r1, [r0, #DMC_MEMCONFIG1]

	ldr     r1, =0xf0000000
	str     r1, [r0, #DMC_PRECHCONFIG]

	ldr     r1, =0x0000040E
	str     r1, [r0, #DMC_TIMINGAREF]

	ldr     r1, =0x0C233286                         @TimingRow      @133MHz
	str     r1, [r0, #DMC_TIMINGROW]

	ldr     r1, =0x12130005
	str     r1, [r0, #DMC_TIMINGDATA]

	ldr     r1, =0x0E100222                         @Timing Power
	str     r1, [r0, #DMC_TIMINGPOWER]

	ldr     r1, =0x07000000                         @chip0 Deselect
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x01000000                         @chip0 PALL
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x05000000                         @chip0 REFA
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x05000000                         @chip0 REFA
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x00000032                         @chip0 MRS
	str     r1, [r0, #DMC_DIRECTCMD]

	ldr     r1, =0x0F0010B0                         @ConControl auto refresh on
	str     r1, [r0, #DMC_CONCONTROL]

	ldr     r1, =0x00100002                         @PwrdnConfig
	str     r1, [r0, #DMC_PWRDNCONFIG]

	ldr     r1, =0x00202100                         @MemControl
	str     r1, [r0, #DMC_MEMCONTROL]
#endif

	mov	pc, lr

/* Below code is for ARM926EJS and ARM1026EJS */
	.globl cleanDCache
cleanDCache:
	mrc	p15, 0, pc, c7, c10, 3	/* test/clean D-Cache */
	bne	cleanDCache
	mov	pc, lr

	.globl cleanFlushDCache
cleanFlushDCache:
	mrc	p15, 0, pc, c7, c14, 3	/* test/cleanflush D-Cache */
	bne	cleanFlushDCache
	mov	pc, lr

	.globl cleanFlushCache
cleanFlushCache:
	mrc	p15, 0, pc, c7, c14, 3	/* test/cleanflush D-Cache */
	bne	cleanFlushCache
	mcr	p15, 0, r0, c7, c5, 0	/* flush I-Cache */
	mov	pc, lr

	.ltorg

