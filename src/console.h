//
// console.h
// Output to the serial port
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2020 Christian Lehner

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

extern void cout( const char* str );
extern void cout_endl();
extern void cout_h(unsigned int v);
extern void cout_d(unsigned int val);

#endif
