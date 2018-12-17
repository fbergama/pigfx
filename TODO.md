# PiGFX
## TODO  List

List of things to improve or add to PiGfx.

## Graphics

* transparent and xor for graphics
* TRS-80 graphic mode (using characters 128-191)
* IN (computer->pigfx) binary sequences
  * set mode
  * draw line
  * draw triangle
  * draw rectangle
  * draw ellipse/circle
  * fill surface
  * send sprite 
  * delete sprite
  * display sprite
  * set sprite position
  * move sprite
  * set color rgb
* OUT (pigfx->computer) binary sequences
  * test sprite collisions
  * get resolution
* bitmapped monochrome sprites

## Terminal (text)

* bitmapped fonts
  * change to use 1 bit by pixels (vs 1 byte currently)
  * prefix in binary file to let pigfx know about the actual format
* enable/disable non scroll/no line break modes (let draw text without scrolling)

## System

* Actual modes and other HD resolutions
* escape sequence to get version, resolution, available memory for sprites, returned as kbd input
* Pi Zero W compatibility (bluetooth driver etc)
* Load fonts from files

2018 Dec 9th
Francis Pierot