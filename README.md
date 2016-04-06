# PiGFX 
## Raspberry Pi graphics card / ANSI terminal emulator

PiGFX is a bare metal code for the Raspberry Pi that implements a basic ANSI
terminal emulator and some primitive graphics functions. It can be driven by
pushing characters to the raspi UART with a basic support for ANSI escape codes
to change foreground/background color, move cursor etc. 
The result is that you can easily add an HDMI display output to your embedded
project without the hassle of directly generate the video signal.

This work is inspired by
[Spencer](https://hackaday.io/project/9567-5-graphics-card-for-homebrew-z80)
whose aim was to create a cheap graphics card and keyboard interface to its
homebrew Z80 computer. PiGFX has the advantage of removing the fuss of having a
full-featured Linux system running on your raspi, dramatically decreasing the
boot time, letting the system being more customizable and, of course, increasing
the fun :) 


## How to run

1. Format an SD-card with FAT32 filesystem.
2. Copy ```bin/kernel.img``` in the root of the SD-card along with the files
   ```start.elf``` and ```bootcode.bin``` that are commonly [distributed with
the raspberry pi](https://github.com/raspberrypi/firmware/tree/master/boot)
3. Insert the card and reboot the pi.

As soon as your raspi is turned on, the message "PIGFX Ready!" should be
displayed as a 640x480 @ 60hz video stream from the HDMI interface. Any data
received from the UART is immediately displayed in a terminal-like fashion.


## Terminal ANSI Codes

The following escape sequences can be used to further control the terminal
behaviour

Code                  | Command
---                   | ---
| _ESC_[?25l          | Cursor invisible
| _ESC_[?25h          | Cursor visible
| _ESC_[H             | Move to 0-0
| _ESC_[s             | Save the cursor position 
| _ESC_[u             | Move cursor to previously saved position 
| _ESC_[_Row_;_Col_H  | Move to _Row_,_Col_
| _ESC_[0K            | Clear from cursor to the end of the line
| _ESC_[1K            | Clear from the beginning of the current line to the cursor 
| _ESC_[2K            | Clear the whole line 
| _ESC_[2J            | Clear the screen and move the cursor to 0-0 
| _ESC_[_n_A          | Move the cursor up _n_ lines
| _ESC_[_n_B          | Move the cursor down _n_ lines
| _ESC_[_n_C          | Move the cursor forward _n_ characters
| _ESC_[_n_D          | Move the cursor backward _n_ characters
| _ESC_[0m            | Reset color attributes (white on black) 
| _ESC_[38;5;_n_m     | Set foreground color to _n_ (0-255) 
| _ESC_[48;5;_n_m     | Set background color to _n_ (0-255) 


Where ```_ESC_``` is the binary character ```0x1B``` and ```_n_```,
```_Row_```, ```_Col_``` is any sequence of numeric characters like ```123```.

See [Here](https://en.wikipedia.org/wiki/File:Xterm_256color_chart.svg) for a
reference of the implemented xterm color palette.


## Compiling



## License

The MIT License (MIT)

Copyright (c) 2016 Filippo Bergamasco.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
