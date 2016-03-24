.global bootstrap
bootstrap:
    mov sp, #0x8000
    bl  entry_point

.global hang
hang: 
    b hang
