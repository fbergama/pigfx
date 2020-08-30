/* See description at https://www.valvers.com/open-software/raspberry-pi/bare-metal-programming-in-c-part-4/ */

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
    _fast_interrupt_h:           .word   hang


;@ See linker script file
.globl bss_start
bss_start: .word __bss_start__

.globl bss_end
bss_end: .word __bss_end__

.globl pheap_space
pheap_space: .word _heap_start

.globl heap_sz
heap_sz: .word heap_size

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
    mov sp, #EXCEPTION_STACK_SIZE

#if RPI > 2
    /* Change to supervisor mode for RPI3, 1&2 already start in supervisor mode */
    mrs     r3,cpsr         ;@ reads the CPU mode register
    bic     r3,r3,#0x1F     ;@ clears the CPU MODE bits (It will be 1A currently if in HYP_MODE) preserving all else
    orr     r3,r3,#0x13     ;@ sets the CPU_MODE bits for SVC_MODE (0x13) with ORR still keeping all the other bits
    msr     spsr_cxsf,r3    ;@ writes that to the spsr_cxsf register so it gets loaded when he calls for the switch
    add     r3,pc,#4        ;@ calculates the address he wants to go into SVC_MODE from the pc (the two opcodes that follow are that long)
    msr     ELR_hyp,r3      ;@ writes that address value to ELR_hyp register
    eret                    ;@ does the elevated return command
#endif

;@"================================================================"
;@ Now setup stack pointers for the different CPU operation modes.
;@"================================================================"
	cps	#0x11				/* set fiq mode */
	ldr	sp, =__FIQ_stack_core0
	cps	#0x12				/* set irq mode */
	ldr	sp, =__IRQ_stack_core0
	cps	#0x17				/* set abort mode */
	ldr	sp, =__abort_stack_core0
	cps	#0x1B				/* set "undefined" mode */
	ldr	sp, =__abort_stack_core0
	cps	#0x1F				/* set system mode */
	ldr	sp, =__SVC_stack_core0

    ;@ Fill BSS with zeros
    ldr   r4, bss_start
    ldr   r9, bss_end
    mov   r5, #0
clear_bss:
    str   r5,[r4]
    add   r4,r4,#4
    cmp   r4,r9
    ble   clear_bss

    /* Call our main function */
    /* The values r0 - r2 from bootloader are preserved */
    ldr r3, =entry_point
    blx r3

.global hang
hang:
    nop
    b hang

