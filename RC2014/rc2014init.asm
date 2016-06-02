ORG 0000h			; start at 0x0000

RST00:      di                  ; disable interrupts
            jp INIT             
            nop
            nop
            nop
            nop                 ; pad to address 0x0008

RST08:      jp TXA

TXA:        push af             ; Store character
conout1:    in a,($80)          ; Status byte
            bit 1,a             ; Set Zero flag if still transmitting character
            jr Z,conout1        ; Loop until flag signals ready
            pop af              ; Retrieve character
            out ($81),a         ; Output the character
            ret

INIT:       ld hl,$80ED
            ld sp,hl

            ld a,$96            ; Initialize ACIA
            out ($80),a
            ; init completed
