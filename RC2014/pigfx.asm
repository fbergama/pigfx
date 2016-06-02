MODULE pigfx

section code

ANSI_START: ld a, 0x1B
            rst $08
            ld a, '['
            rst $08
            ret

public pigfx_print
pigfx_print: 
            ld a, (hl)
            or a
            ret z
            rst $08
            inc hl
            jp pigfx_print

; HL: number to convert
public pigfx_printnum
pigfx_printnum:     
            ld  a, 0
            or h
            or l             ; if the number is 0
            jr  z, PNa       ; print it and exit
            ld  a, 0         ; reset a
            ld  bc, -1000    ; else check powers of 10
            call    PNDOSUB
            ld  bc, -100
            call    PNDOSUB
            ld  c, -10
            call    PNDOSUB
            ld  c, b
            jp      PNDOSUB
PNCOUNT:    inc a
PNDOSUB:    add hl, bc
            jr  c, PNCOUNT
            sbc hl, bc
            or a
            ret z
PNa:        add a, '0'
            rst $08
            ld a, 0
            ret

public pigfx_show_cursor
pigfx_show_cursor:  
            call ANSI_START
            ld hl, cursor_vis_str
            call pigfx_print
            ret

public pigfx_hide_cursor
pigfx_hide_cursor:  
            call ANSI_START
            ld hl, cursor_inv_str
            call pigfx_print
            ret


; Set foreground color 
;
;  HL: color (0-255)
;
public pigfx_fgcol
pigfx_fgcol:
            push hl             ; push color value to stack
            call ANSI_START     ; start sequence
            ld hl, fgcol_str    ; load fgcolor command identifier
            call pigfx_print    ; and print it
            pop hl              ; pop color value
            call pigfx_printnum ; print color value as ascii string
            ld a, 'm'           ; terminate code with 'm'
            rst $08             ;
            ret                 ; end 
            

section data

cursor_inv_str: DEFM "?25l",0
cursor_vis_str: DEFM "?25h",0
fgcol_str:      DEFM "38;5;",0
