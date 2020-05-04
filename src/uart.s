.text

;@ initialize main UART on pin 14 (TX) and 15 (RX)
.global uart_init
uart_init:
    push {r0,r1,r3,lr}

    ;@ set TX to use no resistor, RX to use pull-up resistor
    mov r3, #0x20
    mov r3, r3, LSL #8
    orr r3, r3,  #0x20
    mov r3, r3, LSL #16   ;@ r3 = GPIO_BASE = 0x20200000

    ;@  write 0 to GPPUD (use no resistor)
    mov r0, #0x0
    str r0, [r3, #0x94]  

    ;@ wait 300 cycles
    mov r0, #300
    bl  busywait;

    ;@ set bit 14 in PUDCLK0 (set resistor state for pin 14 - TX)
    mov r0, #0x01
    mov r0, r0, LSL #14
    str r0, [r3, #0x98]  

    ;@ wait 300 cycles
    mov r0, #300
    bl  busywait;

    ;@  write 0 to GPPUD
    mov r0, #0x0
    str r0, [r3, #0x94]  

    ;@ clear PUDCLK0
    mov r0, #0x0
    str r0, [r3, #0x98]  

    ; @  write 2 to GPPUD (use pull-up resistor)
    mov r0, #0x02
    str r0, [r3, #0x94]

    ; @ wait 300 cycles
    mov r0, #300
    bl  busywait

    ; @ set bit 15 in PUDCLK0 (set resistor state for pin 15 - RX)
    mov r0, #0x01
    mov r0, r0, LSL #15
    str r0, [r3, #0x98]

    ; @ wait 300 cycles
    mov r0, #300
    bl  busywait

    ; @  write 0 to GPPUD
    mov r0, #0x0
    str r0, [r3, #0x94]

    ; @ clear PUDCLK0
    mov r0, #0x0
    str r0, [r3, #0x98]

    ;@ set r3 = UART0_BASE = 0x20201000        
    orr r3, r3, #0x1000
    
    ;@ Disable UART0:
    ;@ clear UART0_CR = UART0_BASE + 0x30;
    mov r0, #0x0
    str r0, [r3, #0x30]

    ;@ Clear UART0 interrupts:
    ;@ clear UART0_ICR = UART0_BASE + 0x44;
    mov r0, #0xFFFFFFFF
    str r0, [r3, #0x44]

    ;@ Set 8bit (bit 6-5=1), no parity (bit 7=0), FIFO enable (bit 4=1)
    mov r0, #0x70
    str r0, [r3, #0x2C]  ;@ UART0_LCRH 

    ;@ Enable TX(bit9) RX(bit8) and UART0(bit0)
    mov r0, #0x3
    mov r0, r0, LSL #8
    add r0, r0, #1
    str r0, [r3, #0x30]  ;@ UART0_CR  

    pop {r0,r1,r3,lr}
    bx lr        



;@  Writes to UART0
;@  r0: memory address
;@  r1: data size in bytes
;@ 
.global uart_write
uart_write:
    push {r2,r3,r4,r5,r6}

    ;@ set r3=UART0_BASE=0x20201000
    mov r3, #0x20
    lsl r3, r3, #8
    orr r3, #0x20
    lsl r3, r3, #8
    orr r3, #0x10
    lsl r3, r3, #8
    
    ;@ set r4=UART0_FR=UART0_BASE+0x18
    add r4, r3, #0x18

    mov r6, #4   

write_loop:
    cmp r1, #0
    beq write_done 

    ;@ busy wait if transmit fifo is full
bw: ldr r5, [r4]
    tst r5, #0x20   ;@ Test bit 5
    bne bw

    ;@ load next 4 bytes of data, if needed
    cmp r6, #4
    bne shift_and_send 
    ldr r2, [r0]
    add r0, r0, #4
    mov r6, #0

shift_and_send:
    and r5, r2, #0xFF   ;@ Extract the left-most byte of r2
    str r5, [r3]        ;@ Add to TX FIFO
    lsr r2, r2, #8      ;@ Shift r2 1byte left
    sub r1, r1, #1      ;@ Decrease data size counter
    add r6, r6, #1      ;@ Increase the number of bytes sent for this word
    b   write_loop

write_done:
    
    pop {r2,r3,r4,r5,r6}
    bx lr


;@  Writes a null-terminated string to UART0
;@  r0: memory address
;@ 
.global uart_write_str
uart_write_str:
    push {r0,lr}

    bl   strlen  ;@ strlen wants string address in r0
    mov r1, r0

    pop {r0}
    bl   uart_write

    pop {lr}
    bx lr


;@ Check if data is available in the receive FIFO
;@  r0:  1 if data is available, 0 if not
.global uart_poll
uart_poll:
    push {r3,r4}

    ;@ set r3=UART0_BASE=0x20201000
    mov r3, #0x20
    lsl r3, r3, #8
    orr r3, #0x20
    lsl r3, r3, #8
    orr r3, #0x10
    lsl r3, r3, #8

    mov r0, #0
    ldr r4, [r3,#0x18]  ;@ read from UART0_FR ( UART0_BASE + 0x18)
    tst r4, #0x10       ;@ check if bit4 is set
    bne 1f
    mov r0, #1
    
1:
    pop {r3,r4}
    bx lr
    

;@ Received byte is put in r0
.global uart_read_byte
uart_read_byte:

    push {r3,r4}

    ;@ set r3=UART0_BASE=0x20201000
    mov r3, #0x20
    lsl r3, r3, #8
    orr r3, #0x20
    lsl r3, r3, #8
    orr r3, #0x10
    lsl r3, r3, #8

    ;@ busy wait if receive fifo is empty
bwr: 
    ldr r4, [r3,#0x18]  ;@ read from UART0_FR ( UART0_BASE + 0x18)
    tst r4, #0x10       ;@ check if bit4 is set
    bne bwr

    ldr r0, [r3]        ;@ read from RX fifo
    and r0, #0xFF

    pop {r3,r4}
    bx lr


;@  Reads 2 ascii hex chars from UART0 and returns
;@  its byte value in r0
;@  a number > 255 is returned if error occurred
;@
.global uart_read_hex
uart_read_hex:
    push {r8,lr}
    sub sp,#4
    mov r8,sp   ;@ r8 is the buffer memory pointer

    bl  uart_read_byte
    strb r0, [r8]
    add r8, #1

    bl uart_read_byte
    strb r0, [r8]

    sub r0, r8, #1
    bl  hex2byte

    add sp,#4
    pop {r8,lr}
    bx lr


.global uart_purge
uart_purge:

    push {lr}
2:
    bl uart_poll
    cmp r0, #0
    beq 1f
    bl uart_read_byte
    b 2b
1:
    pop {lr}
    bx lr
    


;@ Dumps a memory area to UART0 
;@  r0: start address
;@  r1: end address
;@
.global uart_dump_mem
uart_dump_mem:
    
    push {r2,r3,r4,r5,r6,r7,r8,lr}
    sub sp, #256
    mov r8, sp

    mov r4,r0      ;@ r4 is the current address
    add r6,r1,#1   ;@ r6 is the end address +1

    cmp r4,r6
    beq 4f      ;@ if curr address == end address, quit

2:
    mov r7,#0   ;@ reset line counter
    mov r5, r8  ;@ reset string buffer  

    mov r0, r4
    mov r1, r5
    bl word2hexstr      ;@ write the current address (r4) to out string (r5)
    add r5, r5, #8      ;@ out string += 8 bytes

    mov r2,#58            ;@ add ':'
    strb r2, [r5]
    add r5, r5, #1      ;@ out string += 1 byte

    mov r2,#32            ;@ add ' '
    strb r2, [r5]
    add r5, r5, #1      ;@ out string += 1 byte

3:
    ldrb r0, [r4]       ;@ read another byte
    mov r1, r5
    bl byte2hexstr   ;@ write the current byte to out string (r5)
    add r5, r5, #2      ;@ out string += 2 bytes
    add r4, r4, #1
    add r7, r7, #1

    mov r2,#32            ;@ add ' '
    strb r2, [r5]
    add r5, r5, #1      ;@ out string += 1 byte

    cmp r4,r6
    beq 1f      ;@ if curr address == end address flush 

    cmp r7, #20
    bne 3b
    
1:  
    mov r2,#10            ;@ add '\n'
    strb r2, [r5]
    add r5, r5, #1      ;@ out string += 1 byte
    mov r2,#0            ;@ add '\0'
    strb r2, [r5]
    add r5, r5, #1      ;@ out string += 1 byte

    push {r0}
    mov r0, r8
    bl uart_write_str
    pop {r0}

    cmp r4,r6
    bne 2b      ;@ if curr address != end address continue, else exit 

4:    
    add sp, #256
    pop {r2,r3,r4,r5,r6,r7,r8,lr}
    bx lr


.global uart_load_ihex
uart_load_ihex:

    push {r0,r1,r2,r4,r5,r6,r9,r10,lr}

    ldr r0, =ihex_ready_str 
    bl uart_write_str

main_loop:
    ;@ clear the checksum register r9
    mov r9, #0

    ;@ wait for start code (ascii colon ':')
wait_cl:
    bl   uart_read_byte
    cmp r0, #58     ;@ check if read byte is ':'
    bne wait_cl

    
    ;@ read byte count (2 chars hex)
    ;@ and save it in r4
    bl  uart_read_hex
    mov r4, r0
    add r9, r9, r4


    ;@ read memory address (4 chars hex)
    ;@ and save it in r5
    bl  uart_read_hex
    cmp r0, #0xFF
    bgt bad_data    
    add r9, r9, r0
    mov r1, r0
    lsl r1,#8
    bl  uart_read_hex
    cmp r0, #0xFF
    bgt bad_data    
    add r9, r9, r0
    orr r5,r1,r0
    mov r10, r5


    ;@ read record type (2 chars hex)
    ;@ and save it in r6
    bl  uart_read_hex
    mov r6, r0
    add r9, r9, r0

    ;@ parse record type
    cmp r6,#0
    beq line_dump ;@ DATA type

    cmp r6,#1
    beq dump_complete ;@ EOF type

    cmp r6,#3
    beq main_loop  ;@ START SEGMENT type (we just ignore it)

    ;@ else..
    b bad_type  

    ;@ read hex data and dump to memory
line_dump:
    cmp r4,#0
    beq line_dump_done
    bl uart_read_hex
    cmp r0, #0xFF
    bgt bad_data    
    add r9, r9, r0
    strb r0,[r5]
    add r5,r5,#1
    sub r4,r4,#1
    b line_dump

line_dump_done:
    ;@  checksum test
    bl uart_read_hex
    add r9, r9, r0
    and r9, r9, #0xFF   ;@ keep the LSB
    cmp r9,#0
    bne bad_checksum

    ;@ debug dump
    mov r0, r10
    sub r1, r5, #1
    bl uart_dump_mem
    
    b main_loop

dump_complete:
    ldr r0, =ihex_loaded_str
    bl uart_write_str
    bl uart_purge
    mov r0, #0xAA
    lsl r0,r0,#8
    bx r0        ;@ JUMP away

bad_type:
    ldr r0, =bad_type_str
    bl uart_write_str
    b alldone

bad_data:
    ldr r0, =bad_data_str
    bl uart_write_str
    b alldone

bad_checksum:
    ldr r0, =bad_checksum_str
    bl uart_write_str
    b alldone

alldone:
    bl uart_purge
    pop {r0,r1,r2,r4,r5,r6,r9,r10,lr}
    bx lr


.balign 4
bad_checksum_str: .asciz "iHEX: BAD CHECKSUM\n"
.balign 4
bad_type_str: .asciz "iHEX: BAD TYPE\n"
.balign 4
bad_data_str: .asciz "iHEX: BAD DATA\n"
.balign 4
ihex_loaded_str: .asciz "iHEX: LOADED, Jumping to 0xAA00\n"
.balign 4
ihex_ready_str: .asciz "\niHEX: RDY\n"
