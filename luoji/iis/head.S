.global _start
_start:
	b reset
	ldr pc, _hang
	ldr pc, _hang
	ldr pc, _hang
	ldr pc, _hang
	ldr pc, _hang
	ldr pc, _hang
	ldr pc, _hang

_hang:
	.word 0x12345678
	
reset:

	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

	ldr	sp, =0xbc008000
	bic	sp, sp, #7 
	bl clean_bss
	bl cpu_init_crit
	bl main

delay:
  ldr r3,=1200000
delay1:
  sub r3, r3, #1
  cmp r3, #0
  bne delay1
  mov pc, lr


cpu_init_crit:
  /*
   * flush D cache before disabling it
   */
  mov r0, #0
flush_dcache:
  mrc p15, 0, r15, c7, c10, 3
  bne flush_dcache

  mcr p15, 0, r0, c8, c7, 0 /* invalidate TLB */
  mcr p15, 0, r0, c7, c5, 0 /* invalidate I Cache */

  /*
   * disable MMU and D cache
   * enable I cache if CONFIG_SYS_ICACHE_OFF is not defined
   */
  mrc p15, 0, r0, c1, c0, 0
  bic r0, r0, #0x00000300 /* clear bits 9:8 (---- --RS) */
  bic r0, r0, #0x00000087 /* clear bits 7, 2:0 (B--- -CAM) */
#ifdef CONFIG_SYS_EXCEPTION_VECTORS_HIGH
  orr r0, r0, #0x00002000 /* set bit 13 (--V- ----) */
#else
  bic r0, r0, #0x00002000 /* clear bit 13 (--V- ----) */
#endif
  orr r0, r0, #0x00000002 /* set bit 2 (A) Align */
#ifndef CONFIG_SYS_ICACHE_OFF
  orr r0, r0, #0x00001000 /* set bit 12 (I) I-Cache */
#endif
  mcr p15, 0, r0, c1, c0, 0

  /*
   * Go setup Memory and board specific bits prior to relocation.
   */
  mov ip, lr    /* perserve link reg across call */
  #bl  lowlevel_init /* go setup pll,mux,memory */
  mov lr, ip    /* restore link */
  mov pc, lr    /* back to my caller */

