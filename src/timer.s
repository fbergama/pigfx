
;@  busy-wait a fixed amount of time in us.
;@ r0: number of micro-seconds to sleep
;@
.global usleep
usleep:
    push {r1,r2,r3}

    mov r1, #0x2000
    lsl r1, #16
    orr r1, #0x3000
    ;@ now r1 = 0x20003000 (system timer address)

    add r3, r1, #0x4   ;@ (r3 = system timer lower 32 bits)
    ldr r2, [r3]

    add r0, r0, r2

1:
    ldr r1, [r3]
    cmp r1, r0
    blt 1b  ;@ busy loop
    

    pop {r1,r2,r3}
    bx lr


.global time_microsec
time_microsec:
    push {r1,r3}

    mov r1, #0x2000
    lsl r1, #16
    orr r1, #0x3000
    ;@ now r1 = 0x20003000 (system timer address)

    add r3, r1, #0x4   ;@ (r3 = system timer lower 32 bits)
    ldr r0, [r3]
    pop {r1,r3}
    bx lr
    
