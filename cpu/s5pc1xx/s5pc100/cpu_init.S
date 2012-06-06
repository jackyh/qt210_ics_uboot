#include <config.h>
#include <s5pc100.h>

	.globl mem_ctrl_asm_init
mem_ctrl_asm_init:

	ldr	r0, =APB_DMC_BASE			@APB_DMC_BASE 0xE6000000

	ldr	r1, =PRO_ID_BASE
	ldr	r2, [r1, #PRO_ID_OFFSET]
	bic	r2, #0xfffffdff
	mov	r2, r2, lsr #9
	cmp	r2, #0x1
	beq	onenand_pop

single:
        ldr     r1, =0x003B3B00                         @Phycontrol0 DLL parameter setting
        str     r1, [r0, #DMC_PHYCONTROL0]

        ldr     r1, =0x00000004                         @Phycontrol1 DLL parameter setting
        str     r1, [r0, #DMC_PHYCONTROL1]

        ldr     r1, =0x00000000                         @Phycontrol2 DLL parameter setting
        str     r1, [r0, #DMC_PHYCONTROL2]

        ldr     r1, =0x003B3B02                         @Dll on
        str     r1, [r0, #DMC_PHYCONTROL0]

        ldr     r1, =0x003B3B03                         @dll start
        str     r1, [r0, #DMC_PHYCONTROL0]

        ldr     r1, =0x6A3B3B03                         @Force Value locking
        str     r1, [r0, #DMC_PHYCONTROL0]

        ldr     r1, =0x6A3B3B09                         @Dll off
        str     r1, [r0, #DMC_PHYCONTROL0]

        ldr     r1, =0xFF001010                         @auto refresh off
        str     r1, [r0, #DMC_CONCONTROL]

        ldr     r1, =0xFF212100                         @Dll off
        str     r1, [r0, #DMC_MEMCONTROL]

#if defined(MEMORY_SIZE_256)
        ldr     r1, =0x20F01322
        str     r1, [r0, #DMC_MEMCONFIG0]

        ldr     r1, =0x40F00322
        str     r1, [r0, #DMC_MEMCONFIG1]

        ldr     r1, =0x20000000
        str     r1, [r0, #DMC_PRECHCONFIG]

	ldr     r1, =0x0000050E
	str     r1, [r0, #DMC_TIMINGAREF]
#else
        ldr     r1, =0x20F81312
        str     r1, [r0, #DMC_MEMCONFIG0]

        ldr     r1, =0x40F80312
        str     r1, [r0, #DMC_MEMCONFIG1]

        ldr     r1, =0x20000000
        str     r1, [r0, #DMC_PRECHCONFIG]

	ldr     r1, =0x0000004E
        str     r1, [r0, #DMC_TIMINGAREF]
#endif
        ldr     r1, =0x0C233287                         @TimingRow      @133MHz
        str     r1, [r0, #DMC_TIMINGROW]

        ldr     r1, =0x12130005
        str     r1, [r0, #DMC_TIMINGDATA]

        ldr     r1, =0x0E120122                         @Timing Power
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

        ldr     r1, =0x07100000                         @chip1 Deselect
        str     r1, [r0, #DMC_DIRECTCMD]

        ldr     r1, =0x01100000                         @chip1 PALL
        str     r1, [r0, #DMC_DIRECTCMD]

        ldr     r1, =0x05100000                         @chip1 REFA
        str     r1, [r0, #DMC_DIRECTCMD]

        ldr     r1, =0x05100000                         @chip1 REFA
        str     r1, [r0, #DMC_DIRECTCMD]

        ldr     r1, =0x00100032                         @chip1 MRS
        str     r1, [r0, #DMC_DIRECTCMD]

#if defined(MEMORY_SIZE_256)
        ldr     r1, =0xFF0010B0                         @ConControl auto refresh on
        str     r1, [r0, #DMC_CONCONTROL]

        ldr     r1, =0x001000ff                         @PwrdnConfig
        str     r1, [r0, #DMC_PWRDNCONFIG]

        ldr     r1, =0xFF212113                         @MemControl
        str     r1, [r0, #DMC_MEMCONTROL]
#else
        ldr     r1, =0xFF001030                         @ConControl auto refresh on
        str     r1, [r0, #DMC_CONCONTROL]

        ldr     r1, =0x00100002                         @PwrdnConfig
        str     r1, [r0, #DMC_PWRDNCONFIG]

        ldr     r1, =0xFF212100                         @MemControl
        str     r1, [r0, #DMC_MEMCONTROL]
#endif

        b	exit_cpu_init
onenand_pop:
	ldr	r1, =0x50101000				@Phycontrol0 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x000000F4				@Phycontrol1 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL1]

	ldr	r1, =0x00000000				@Phycontrol2 DLL parameter setting
	str	r1, [r0, #DMC_PHYCONTROL2]

	ldr	r1, =0x50101002				@Dll on
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x50101003				@dll start
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x50101003				@Force Value locking
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0x50101009				@Dll off
	str	r1, [r0, #DMC_PHYCONTROL0]

	ldr	r1, =0xFF001010				@auto refresh off
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =0x00212100				@Dll off
	str	r1, [r0, #DMC_MEMCONTROL]

	ldr	r1, =0x28F80222
	str	r1, [r0, #DMC_MEMCONFIG0]

	ldr	r1, =0x20F80222
	str	r1, [r0, #DMC_MEMCONFIG1]

	ldr	r1, =0x20000000
	str	r1, [r0, #DMC_PRECHCONFIG]

	ldr	r1, =0x0000050E
	str	r1, [r0, #DMC_TIMINGAREF]

	ldr	r1, =0x0C233287				@TimingRow	@133MHz
	str	r1, [r0, #DMC_TIMINGROW]

	ldr	r1, =0x32330303
	str	r1, [r0, #DMC_TIMINGDATA]

	ldr	r1, =0x04141433				@Timing Power
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

	ldr	r1, =0xFF002030				@ConControl auto refresh on
	str	r1, [r0, #DMC_CONCONTROL]

	ldr	r1, =0x00100002				@PwrdnConfig
	str	r1, [r0, #DMC_PWRDNCONFIG]

	ldr	r1, =0xFF212100				@MemControl
	str	r1, [r0, #DMC_MEMCONTROL]

	b	exit_cpu_init

exit_cpu_init:
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

