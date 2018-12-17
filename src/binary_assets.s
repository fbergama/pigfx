.section .rodata

.global G_FONT8X8_GLYPHS
.align 4
G_FONT8X8_GLYPHS: .incbin "fonts/font8x8.bin"

.global G_FONT8X24_GLYPHS
.align 4
G_FONT8X24_GLYPHS: .incbin "fonts/TRSfont8x24.bin"

.global G_FONT8X16_GLYPHS
.align 4
G_FONT8X16_GLYPHS: .incbin "fonts/TRSfont8x16.bin"

#if ENABLED(RC2014)
.global G_STARTUP_LOGO
.align 4
G_STARTUP_LOGO: .incbin "sprite/rc2014logo.bin"
#endif
