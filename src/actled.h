//
// actled.h
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2014-2020 Christian Lehner
// Based on the leiradel tutorial at
// https://github.com/leiradel/barebones-rpi/blob/master/barebones08/rpi/led.c

#ifndef ACTLED_H__
#define ACTLED_H__

extern void (*led_set)(const int on);

int led_init(board_t board);

#endif /* ACTLED_H__ */
