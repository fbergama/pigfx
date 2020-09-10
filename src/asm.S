/* This code is borrowed from the circle project and modified to fit PiGFX */
/* 2020 Christian Lehner */

#include "memory.h"
#include "exception.h"

.global bootstrap
bootstrap:
    ldr pc, _reset_h
    ldr pc, _undefined_instruction_h
    ldr pc, _software_interrupt_h
    ldr pc, _prefetch_abort_h
    ldr pc, _data_abort_h
    ldr pc, _unused_handler_h
    ldr pc, _interrupt_h
    ldr pc, _fast_interrupt_h

_reset_h:                        .word   _reset_
    _undefined_instruction_h:    .word   UndefinedInstructionStub
    _software_interrupt_h:       .word   hang
    _prefetch_abort_h:           .word   PrefetchAbortStub
    _data_abort_h:               .word   DataAbortStub
    _unused_handler_h:           .word   hang
    _interrupt_h:                .word   irq_handler_
    _fast_interrupt_h:           .word   fiq_handler_

/* The bootloader starts, loads are executable, and enters */
/* execution at 0x8000 with the following values set.      */
/* r0 = boot method (usually 0 on pi)       		   */
/* r1 = hardware type (usually 0xc42 on pi) 		   */
/* r2 = start of ATAGS ARM tag boot info (usually 0x100)   */

;@ Initial entry point
_reset_:
    /* Copy the vector table (top of this file) to the active table at 0x00000000 */
    mov     r3, #0x8000
    mov     r4, #0x0000
    ldmia   r3!,{r5, r6, r7, r8, r9, r10, r11, r12}
    stmia   r4!,{r5, r6, r7, r8, r9, r10, r11, r12}
    ldmia   r3!,{r5, r6, r7, r8, r9, r10, r11, r12}
    stmia   r4!,{r5, r6, r7, r8, r9, r10, r11, r12}

    /* Force SVC Mode and mask interrupts */
	mrs	r3 , cpsr
	eor	r3, r3, #0x1A		/* test for HYP mode */
	tst	r3, #0x1F
	bic	r3 , r3 , #0x1F		/* clear mode bits */
	orr	r3 , r3 , #0xC0 | 0x13	/* mask IRQ/FIQ bits and set SVC mode */
	bne	1f				/* branch if not HYP mode */
	orr	r3, r3, #0x100		/* mask Abort bit */
	adr	lr, 2f
	msr	spsr_cxsf, r3
	.word	0xE12EF30E			/* msr ELR_hyp, lr */
	.word	0xE160006E			/* eret */
1:	msr	cpsr_c, r3
2:

;@"================================================================"
;@ Now setup stack pointers for the different CPU operation modes.
;@"================================================================"
	cps	#0x11				/* set fiq mode */
	ldr	sp, =MEM_FIQ_STACK
	cps	#0x12				/* set irq mode */
	ldr	sp, =MEM_IRQ_STACK
	cps	#0x17				/* set abort mode */
	ldr	sp, =MEM_ABORT_STACK
	cps	#0x1B				/* set "undefined" mode */
	ldr	sp, =MEM_ABORT_STACK
	cps	#0x1F				/* set system mode */
	ldr	sp, =MEM_KERNEL_STACK
	b	entry_point

.global hang
hang:
    nop
    b hang

