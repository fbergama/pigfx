//
// prop.h
// Get mailbox properties
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2020 Christian Lehner
// Based on the leiradel tutorial at
// https://github.com/leiradel/barebones-rpi

#ifndef PROP_H__
#define PROP_H__

#include <stdint.h>

#define PROP_CLOCK_EMMC   1
#define PROP_CLOCK_UART   2
#define PROP_CLOCK_ARM    3
#define PROP_CLOCK_CORE   4
#define PROP_CLOCK_V3D    5
#define PROP_CLOCK_H264   6
#define PROP_CLOCK_ISP    7
#define PROP_CLOCK_SDRAM  8
#define PROP_CLOCK_PIXEL  9
#define PROP_CLOCK_PWM   10

typedef struct
{
    unsigned int baseAddr;
    unsigned int size;
} tSysRam;

uint32_t   prop_revision(void);
uint32_t   prop_fwrev(void);
uint32_t   prop_model(void);
int        prop_macaddr(unsigned char* pOutAddr);
uint64_t   prop_serial(void);
void       prop_VCRAM(tSysRam* ram);
void       prop_ARMRAM(tSysRam* ram);

#endif /* PROP_H__ */
