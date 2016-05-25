.section .rodata

.global G_FONT_GLYPHS
.align 4
G_FONT_GLYPHS: .incbin "fonts/font8x8.bin"

.global G_STARTUP_LOGO
.align 4
G_STARTUP_LOGO: .incbin "sprite/rc2014logo.bin"
