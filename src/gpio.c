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

unsigned char gpio_get(const unsigned pin)
{
    const unsigned index = pin >> 5;
    const uint32_t bit = UINT32_C(1) << (pin & 31);
    
    unsigned int gpio_state;
    
    if (index)
    {
        gpio_state = R32(GPIO_GPLEV1);
    }
    else
    {
        gpio_state = R32(GPIO_GPLEV0);
    }
    
    return ((gpio_state & bit) != 0);
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

void gpio_clear_irq(const unsigned pin)
{
  const unsigned index = pin >> 5;
  const uint32_t bit = UINT32_C(1) << (pin & 31);
  unsigned int GPEDS = GPIO_GPEDS0 + index*4;
  unsigned int regVal;
  
  // clear pending flag for this pin
  regVal = R32(GPEDS);
  regVal = regVal | bit;
  W32(GPEDS, regVal);
}

void gpio_setedgedetect(const unsigned pin, const unsigned char edgedetect)
{
  const unsigned index = pin >> 5;
  const uint32_t bit = UINT32_C(1) << (pin & 31);
  unsigned int offset = index*4;
  
  unsigned int GPREN = GPIO_GPREN0 + offset;
  unsigned int GPFEN = GPIO_GPFEN0 + offset;
  unsigned int GPAREN = GPIO_GPAREN0 + offset;
  unsigned int GPAFEN = GPIO_GPAFEN0 + offset;
  
  unsigned int regVal;
  
  // set rising edge
  regVal = R32(GPREN);
  if (edgedetect & GPIO_EDGE_DETECT_RISING) regVal = regVal | bit;
  else regVal = regVal & ~bit;
  W32(GPREN, regVal);
  
  // set falling edge
  regVal = R32(GPFEN);
  if (edgedetect & GPIO_EDGE_DETECT_FALLING) regVal = regVal | bit;
  else regVal = regVal & ~bit;
  W32(GPFEN, regVal);
  
  // set async rising edge
  regVal = R32(GPAREN);
  if (edgedetect & GPIO_EDGE_DETECT_ASYNC_RISING) regVal = regVal | bit;
  else regVal = regVal & ~bit;
  W32(GPAREN, regVal);
  
  // set async falling edge
  regVal = R32(GPAFEN);
  if (edgedetect & GPIO_EDGE_DETECT_ASYNC_FALLING) regVal = regVal | bit;
  else regVal = regVal & ~bit;
  W32(GPAFEN, regVal);
  
  // clear pending flag for this pin
  gpio_clear_irq(pin);
}
 
