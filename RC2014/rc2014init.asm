;@---------------------------------------------------------------------
;@  RC2014 Initialization and I/O routines
;@      Filippo Bergamasco 2016
;@
;@
;@  See http://rc2014.co.uk for schematics and description
;@  of the RC2014 homebrew computer
;@
;@---------------------------------------------------------------------

ORG 0000h			; start at 0x0000

RST00:      di                  ; disable interrupts
            jp INIT             
            nop
            nop
            nop
            nop                 ; pad to address 0x0008

RST08:      jp TX

TX:         ld b, a             ; store in b the caracter to output
txbusy:     in a,($80)          ; read serial status
            bit 1,a             ; check status bit 1
            jr z, txbusy        ; loop if zero (serial is busy)
            ld a, b
            out ($81), a        ; transmit the character
            ret

;@---------------------------------------------------------------------
;@ rc2014_getc 
;@
;@  wait for system UART and return the received character in HL
;@
;@---------------------------------------------------------------------
public rc2014_getc
rc2014_getc:         
rxempty:    in a, ($80)
            bit 0, a
            jr z, rxempty
            in a, ($81)
            ld h, 0
            ld l, a
            ret
            

;@---------------------------------------------------------------------
;@ rc2014_putc 
;@
;@ output the byte in register L to system UART
;@
;@---------------------------------------------------------------------
public rc2014_putc
rc2014_putc:
            ld a, l
            rst $08
            ret


;@---------------------------------------------------------------------
;@ rc2014_putc 
;@
;@ polls the uart receive buffer status and 
;@ returns the result in the register L:
;@   L=0 : no data available
;@   L=1 : data available
;@
;@---------------------------------------------------------------------
public rc2014_pollc
rc2014_pollc:
            ld l, 0
            in a, ($80)
            bit 0, a
            ret z
            ld l, 1
            ret


INIT:       ld hl,$80ED         ; stack initialization
            ld sp,hl

            ld a, $95           ; Initialize ACIA
            out ($80),a

            ; init completed


