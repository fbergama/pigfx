# buildfont - build a binary font file from a PNG for PiGfx

This tool extracts a fixed-width font characters bitmaps from a PNG file displaying all characters and put them in binary form which can be included into PiGfx.

## PNG font format

The PNG file must display characters in sequential format, one after the other, starting at code 0 and advancing horizontally then vertically. 

The tool receives the font width and height. The PNG must use this width and height for its own format. Characters can be displayed in one or more lines but ***each character line must have the height of the font height and a width multiple of the font width***. 

Example: the TRS-80 8x24 font PNG file displays 5 lines of 64 characters. The font width is 8 pixels so the PNG width is 64 x 8 = 512 pixels. The font height is 24 lines so the PNG has at least 120 lines. In fact it has 384 lines because it's a 64 x 16 characters TRS-80 screen copy but the lines after the 120th are ignored.

By default, the tool considers all black pixels as ON (*foreground*) and other colors as OFF (*background*) in the characters (black on white background). Optionally the reverse can be used (white on black background). In Pigfx, the *foreground* and *background* character dots are displayed or not depending on the drawing mode.

Only the 256 first characters found within the format sizes will be stored in the BIN output file and the rest of the PNG will be ignored.

## Building the tool

The makefile will build the tool itself and then the available fonts (BIN from PNG file).

## Using the tool

The tool will load the PNG file using ImageMagick, so you must check that the `convert` utility can be found from command line. To install ImageMagick, use MacPort or Brew

On a Mac, starting a program from the Finder doesn't gives it access to system path, so be sure to use a terminal command line before calling `make` or ImageMagick might not be found.

To get information about the tool, use `buildfont -?`.

To build a BIN file from a PNG file use the following syntaxx:

`buildfont -i <PNGpath> -o <BINpath> -w <fontwidth> -h <fontheight> [-c 0|1] [-q]`

Where :

* `<PNGpath>` is the full or relative path to the PNG file
* `<BINpath>` is the full or relative path to the file which the tool will create
* `<fontwidth` is the pixel width of each character
* `<fontheight`` is the number of lines of each character
* `-c 0` means the PNG displays black characters on any non-black color background
* `-c 1` means the PNG displays any non-black color characters on black background 
* `-q` is for quiet mode, by default the tool displays an ASCII rendering of each character.

When run, the tool displays an ASCII rendering of each character with its code on the standard output.

## Remarks

Original program for the 8x8 font used in the original PiGfx by Filippo Bergamasco.

Extension to other files and formats by Francis Piérot.

8x8 font downloaded from:
[http://uzebox.org/wiki/index.php?title=Font_Bitmaps]()

8x24 TRS-80 font made via screen copy of:
[http://48k.ca/320char.html]()

8x16 TRS-80 Font photoshoped from 8x24. Minor artifacts: graphics characters (2x3 pixels as codes 128-191) are using 5 lines for top pixels, 6 lines for middle pixels and 5 lines for bottom pixels so they fill the 16 lines but the middle pixels are a little taller than the top and bottom. Some characters do not have separating empty bottom line(s) and hence will touch the top of some charecters below them.