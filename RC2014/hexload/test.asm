SECTION INIT
ORG     $8000 ; 0xFFFF - 2048

start:      di              ; disable interrupts
            ld hl, $FFFF    ; set new stack location
            ld sp, hl       ; to $FFFF

mainf:      ld hl, initstr
            call print

hang:
            nop
            jp hang

TX:         push af             
txbusy:     in a,($80)          ; read serial status
            bit 1,a             ; check status bit 1
            jr z, txbusy        ; loop if zero (serial is busy)
            pop af
            out ($81), a        ; transmit the character
            ret

print: 
            ld a, (hl)
            or a
            ret z
            call TX
            inc hl
            jp print

initstr:            DEFM "IT WORKS.",10,13,0
