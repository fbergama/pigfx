# PiGFX
## 2018 Modifications by Francis PiÃ©rot

The following have been done on the RC2014 branch of PiGFX.

- Extended the buildfont tool (see the [fonts](fonts/) directory)
- Added 8x16 and 8x24 TRS-80 fonts
- Added `ESC[=xxh` (legacy PC ANSI.SYS) to change display mode (actually only implements 640x480x8 and 320x200x8)
- Added `ESC[=0f` for 8x8 font
- Added `ESC[=1f` for 8x16 font
- Added `ESC[=2f` for 8x24 font
- Added `ESC[=0m` for normal mode (sprites and characters)
- Added `ESC[=1m` for XOR mode (sprites and characters)
- Added `ESC[=2m` for TRANSPARENT mode (sprites and characters)
- Added `ESC[=xxt` to set tabulation width (8 by default)
- Initial display draws a map of colors and the transparent RC2014 logo over it

The following have been done in C sources.

- Constants for colors 0-15 (DRAWING_COLOR enum, can be used where GFX_COL is used)
- Function pointers for drawing-mode related operations (sprite and character)
- Varous comments and function headers
- Precomputed constants based on font width and height to avoid calculations at drawing time
- Moved scn_state structure in a separate header
 
I have some work in progress on graphic functions and communication mode with the RC2014.

Any help to implement actual display modes other than 320x200x8 and 640x480x8 would be great.

2018 Nov 5th
Francis Pierot