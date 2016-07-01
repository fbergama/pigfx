# PiGFX for RC2014 homebrew computer

This subdirectory contains all the software written for the 
[RC2014 homebrew computer](http://rc2014.co.uk). 

At the moment is composed by the following components:

- The RC2014 initialization code for ROM/RAM system (```init/``` directory)
- PiGFX interface library (```pigfx/``` directory)
- [TinyBasicPlus](https://github.com/BleuLlama/TinyBasicPlus) port, originally written by Scott Lawrence and adapted for the Z80/RC2014 
- A simple snake game (in the ```snake/``` directory)

## How to build

- Download and install the [z88dk development kit](http://www.z88dk.org/forum)
- Export the ```ZCCCFG``` environment variable (Note that the C is repeated 3 times) to the ```<z88dk_root>/lib/config``` directory
- Run ```make```


## How to run

The generated binary rom files can be directly burned to the RC2014 eeprom or
executed with the [RC2014 emulator](https://github.com/BleuLlama/z80pack/tree/master/rc2014sim).


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
