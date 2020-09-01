//
// board.h
// Get the board type we're running on
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2014-2020 Christian Lehner
// Based on the leiradel tutorial: https://github.com/leiradel/barebones-rpi/blob/master/barebones08/rpi/board.c

#ifndef BOARD_H__
#define BOARD_H__

#include <stdint.h>

typedef enum {
  BOARD_MODEL_A = 0,
  BOARD_MODEL_B = 1,
  BOARD_MODEL_A_PLUS = 2,
  BOARD_MODEL_B_PLUS = 3,
  BOARD_MODEL_2B = 4,
  BOARD_MODEL_ALPHA = 5,
  BOARD_MODEL_CM1 = 6,
  BOARD_MODEL_3B = 8,
  BOARD_MODEL_ZERO = 9,
  BOARD_MODEL_CM3 = 10,
  BOARD_MODEL_ZERO_W = 12,
  BOARD_MODEL_3B_PLUS = 13,
  BOARD_MODEL_3A_PLUS = 14,
  BOARD_MODEL_CM3_PLUS = 16,
  BOARD_MODEL_4B = 17
}
board_model_t;

typedef enum {
  BOARD_PROCESSOR_BCM2835 = 0,
  BOARD_PROCESSOR_BCM2836 = 1,
  BOARD_PROCESSOR_BCM2837 = 2,
  BOARD_PROCESSOR_BCM2711 = 3
}
board_processor_t;

typedef enum {
  BOARD_MANUFACTURER_QISDA = -1,
  BOARD_MANUFACTURER_SONY_UK = 0,
  BOARD_MANUFACTURER_EGOMAN = 1,
  BOARD_MANUFACTURER_EMBEST = 2,
  BOARD_MANUFACTURER_SONY_JAPAN = 3,
  BOARD_MANUFACTURER_STADIUM = 5
}
board_manufacturer_t;

typedef struct {
  board_model_t model;
  board_processor_t processor;
  int rev_major;
  int rev_minor;
  unsigned ram_mb;
  board_manufacturer_t manufacturer;
}
board_t;

board_t board_info(const uint32_t revision);

const char* board_model(const board_model_t model);
const char* board_processor(const board_processor_t processor);
const char* board_manufacturer(const board_manufacturer_t manufacturer);

#endif /* BOARD_H__ */
