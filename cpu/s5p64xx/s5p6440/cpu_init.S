#include <config.h>
#include <s5p6440.h>

	.globl mem_ctrl_asm_init
mem_ctrl_asm_init:

#ifdef SMDK6440_EVT1
	ldr	r0, =0xE03081D4				@Set DRV STRENTH
	ldr	r1, =0x0FFFFFFF
	str	r1, [r0]
#else
	ldr	r0, =0xE03081D4				@Set DRV STRENTH
	ldr	r1, =0x0
	str	r1, [r0]
#endif	

#ifdef SMDK6440_DDR2
	/* DLL Enable(100MHz ~) */
	ldr	r0, =ELFIN_DMC0_BASE			@APB_DMC_BASE 0xE6000000

	ldr	r1, =0x6A10100B				@PhyControl0	DLL parameter setting, forcing locking value for DLL off
	str	r1, [r0, #PHYCONTROL0_OFFSET]

	ldr	r1, =0x000084F6				@PhyControl1	DLL parameter setting
	str	r1, [r0, #PHYCONTROL1_OFFSET]

	ldr	r1, =0x00000000				@PhyControl2	DLL parameter setting
	str	r1, [r0, #PHYCONTROL2_OFFSET]

/*****************************************************************/
	ldr	r1, =0x0FFF1010				@ConControl	auto refresh off, rd_fetch=1
	str	r1, [r0, #CONCONTROL_OFFSET]

	ldr	r1, =0x00202400				@MemControl	BL=4, 1 chip, DDR2 type, dynamic self refresh, force precharge, dynamic power down off
	str	r1, [r0, #MEMCONTROL_OFFSET]

	ldr	r1, =0x20F00313				@MemConfig0	Col:10, Row:13, Banks:8, 1Gb=64Mbx16bits, (64Mbx16bits) 2°³ => 64Mbx32bits
	str	r1, [r0, #MEMCONFIG0_OFFSET]

	ldr	r1, =0x20000000				@PrechConfig	Chip 0 open page policy(for all banks)
	str	r1, [r0, #PRECHCONFIG_OFFSET]

	ldr	r1, =0x00100004				@PwrdnConfig
	str	r1, [r0, #PWRDNCONFIG_OFFSET]	

/*****************************************************************/
	ldr	r1, =0x0000040D				@TimingAref	7.8us*133MHz=1037(0x40D)
	str	r1, [r0, #TIMINGAREF_OFFSET]

	ldr	r1, =0x11122206				@TimingRow 	- Defalut value
	str	r1, [r0, #TIMINGROW_OFFSET]

	ldr	r1, =0x23140304				@TimingData	CL=4, WL=3(WL=RL-1) => AL=0,  RL=4(RL=CL+AL), ¡Ú if Twtr=14.93ns then OK!
	str	r1, [r0, #TIMINGDATA_OFFSET]

	ldr	r1, =0x051B0232				@TimingPower  	- Defalut value
	str	r1, [r0, #TIMINGPOWER_OFFSET]

/*****************************************************************/
	ldr	r1, =0x07000000				@DirectCmd	chip0 Deselect
	str	r1, [r0, #DIRECTCMD_OFFSET]	

	ldr	r1, =0x01000000				@DirectCmd	chip0 PALL
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00020000				@DirectCmd	chip0 EMRS2
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00030000				@DirectCmd	chip0 EMRS3
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00010400				@DirectCmd	chip0 EMRS1 (MEM DLL on, DQS# disable)
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00000542				@DirectCmd	chip0 MRS (MEM DLL reset) CL=4, BL=4
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x01000000				@DirectCmd	chip0 PALL
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x05000000				@DirectCmd	chip0 REFA
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x05000000				@DirectCmd	chip0 REFA
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00000442				@DirectCmd	chip0 MRS (MEM DLL unreset)
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00010780				@DirectCmd	chip0 EMRS1 (OCD default)
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00010000				@DirectCmd	chip0 EMRS1 (OCD exit, Differential DQS Enable)
	str	r1, [r0, #DIRECTCMD_OFFSET]

/*****************************************************************/
	ldr	r1, =0x0FFF10B0				@DirectCmd	chip0 auto refresh on, PHY Driving Enable
	str	r1, [r0, #CONCONTROL_OFFSET]

	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

	mov	pc, lr
#else
	/* DLL Enable(100MHz ~) */
	ldr	r0, =ELFIN_DMC0_BASE			@APB_DMC_BASE 0xE6000000

	ldr	r1, =0x6A101003				@PhyControl0	DLL parameter setting, forcing locking value for DLL off
	str	r1, [r0, #PHYCONTROL0_OFFSET]

	ldr	r1, =0x000084F4				@PhyControl1	DLL parameter setting
	str	r1, [r0, #PHYCONTROL1_OFFSET]

	ldr	r1, =0x00000000				@PhyControl2	DLL parameter setting
	str	r1, [r0, #PHYCONTROL2_OFFSET]

/*****************************************************************/
	ldr	r1, =0x0FFF1010				@ConControl	auto refresh off, rd_fetch=1
	str	r1, [r0, #CONCONTROL_OFFSET]

	ldr	r1, =0x00202100				@MemControl	BL=4, 1 chip, DDR2 type, dynamic self refresh, force precharge, dynamic power down off
	str	r1, [r0, #MEMCONTROL_OFFSET]

	ldr	r1, =0x00100004				@PwrdnConfig
	str	r1, [r0, #PWRDNCONFIG_OFFSET]	

	ldr	r1, =0x20F00322				@MemConfig0	Col:10, Row:13, Banks:8, 1Gb=64Mbx16bits, (64Mbx16bits) 2°³ => 64Mbx32bits
	str	r1, [r0, #MEMCONFIG0_OFFSET]

	ldr	r1, =0x20000000				@PrechConfig	Chip 0 open page policy(for all banks)
	str	r1, [r0, #PRECHCONFIG_OFFSET]

/*****************************************************************/
	ldr	r1, =0x0000040D				@TimingAref	7.8us*133MHz=1037(0x40D)
	str	r1, [r0, #TIMINGAREF_OFFSET]

	ldr	r1, =0x0A233246				@TimingRow 	- Defalut value
	str	r1, [r0, #TIMINGROW_OFFSET]

	ldr	r1, =0x12130204				@TimingData	CL=4, WL=3(WL=RL-1) => AL=0,  RL=4(RL=CL+AL), ¡Ú if Twtr=14.93ns then OK!
	str	r1, [r0, #TIMINGDATA_OFFSET]

	ldr	r1, =0x0E100122				@TimingPower  	- Defalut value
	str	r1, [r0, #TIMINGPOWER_OFFSET]

/*****************************************************************/
	ldr	r1, =0x07000000				@DirectCmd	chip0 Deselect
	str	r1, [r0, #DIRECTCMD_OFFSET]	

	ldr	r1, =0x01000000				@DirectCmd	chip0 PALL
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x05000000				@DirectCmd	chip0 EMRS2
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x05000000				@DirectCmd	chip0 EMRS3
	str	r1, [r0, #DIRECTCMD_OFFSET]
	
	ldr	r1, =0x00000032				@DirectCmd	chip0 MRS (MEM DLL reset) CL=4, BL=4
	str	r1, [r0, #DIRECTCMD_OFFSET]

	ldr	r1, =0x00020000 			@DirectCmd	chip0 PALL
	str	r1, [r0, #DIRECTCMD_OFFSET]

/*****************************************************************/
	ldr	r1, =0x0FFF10B0				@ConControl	auto refresh on
	str	r1, [r0, #CONCONTROL_OFFSET]

	mov	r1, #0x10000
1:	subs	r1, r1, #1
	bne	1b

	mov	pc, lr
#endif

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

