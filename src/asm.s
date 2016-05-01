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
    _undefined_instruction_h:    .word   /*undefined_instruction_*/ hang 
    _software_interrupt_h:       .word   /*software_interrupt_*/    hang
    _prefetch_abort_h:           .word   /*prefetch_abort_*/        hang
    _data_abort_h:               .word   /*data_abort_*/            hang
    _unused_handler_h:           .word   hang
    _interrupt_h:                .word   irq_handler_ 
    _fast_interrupt_h:           .word   /*fast_interrupt_handler*/ hang

_reset_:
    mov     r0, #0x8000
    mov     r1, #0x0000
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
    mov sp, #0x8000

    ;@ (PSR_IRQ_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xD2
    msr cpsr_c,r0
    mov sp,#0x8000

    ;@ (PSR_FIQ_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xD1
    msr cpsr_c,r0
    mov sp,#0x4000

    ;@ (PSR_SVC_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xD3
    msr cpsr_c,r0
    mov sp,#0x8000000

    bl  entry_point

.global hang
hang: 
    b hang

