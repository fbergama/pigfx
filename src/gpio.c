// This file was used from https://github.com/leiradel/barebones-rpi and modified by Christian Lehner
#include <stdint.h>
#include "gpio.h"
#include "peri.h"
#include "utils.h"

void gpio_select(const unsigned pin, const gpio_function_t mode) {
  // The register index starting at GPFSEL0.
  const unsigned index = (pin * 0xcdU) >> 11;
  // Amount to shift to get to the required pin bits.
  const unsigned shift = (pin - index * 10) * 3;

  // Calculate the GPFSEL register that contains the configuration for
  // the pin.
  const uint32_t gpfsel = GPIO_FSEL0 + index * 4;

  // Read the register.
  const uint32_t value = R32(gpfsel);

  // Set the desired function for the pin.
  const uint32_t masked = value & ~(UINT32_C(7) << shift);
  const uint32_t fsel = masked | mode << shift;

  // Write the value back to the register.
  W32(gpfsel, fsel);
}

void gpio_set(const unsigned pin, const int high) {
  // The register index starting at GPSET0 or GPCLR0.
  const unsigned index = pin >> 5;
  // The bit in the registers to set or clear the pin.
  const uint32_t bit = UINT32_C(1) << (pin & 31);

  if (high) {
    // Write the bit to GPSEL to set the pin high.
    W32(GPIO_SET0 + index * 4, bit);
  }
  else {
    // Write the bit to GPCLR to set the pin low.
    W32(GPIO_CLR0 + index * 4, bit);
  }
}

static void wait(unsigned count) {
  // Spend CPU cycles.
  while (count-- != 0) {
    // Empty.
  }
}

void gpio_setpull(const unsigned pin, const gpio_pull_t pull) {
  const unsigned index = pin >> 5;
  const uint32_t bit = UINT32_C(1) << (pin & 31);

  // Set GPPUD to the desired pull up/down.
  W32(GPIO_PUD, pull);
  // Spend cycles.
  wait(150);

  // Program the pull up/down mode using GPPUDCLK.
  if (index == 0) {
    // Set the pin bit in GPPUDCLK0.
    W32(GPIO_PUDCLK0, bit);
    // Spend cycles.
    wait(150);
    // Clear the bit.
    W32(GPIO_PUDCLK0, 0);
  }
  else {
    // Same thing, but use GPPUDCLK1.
    W32(GPIO_PUDCLK1, bit);
    wait(150);
    W32(GPIO_PUDCLK1, 0);
  }
}
 
