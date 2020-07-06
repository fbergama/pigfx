// This file was used from here: https://github.com/leiradel/barebones-rpi/

#ifndef GPIO_H__
#define GPIO_H__

typedef enum {
  GPIO_INPUT      = 0,
  GPIO_OUTPUT     = 1,
  GPIO_FUNCTION_0 = 4,
  GPIO_FUNCTION_1 = 5,
  GPIO_FUNCTION_2 = 6,
  GPIO_FUNCTION_3 = 7,
  GPIO_FUNCTION_4 = 3,
  GPIO_FUNCTION_5 = 2
}
gpio_function_t;

typedef enum {
  GPIO_PULL_OFF  = 0,
#if RPI<4
  GPIO_PULL_DOWN = 1,
  GPIO_PULL_UP   = 2
#else
  // they must have been on drugs when designing this for the PI4
  GPIO_PULL_UP   = 1,
  GPIO_PULL_DOWN = 2
#endif
}
gpio_pull_t;

#define GPIO_EDGE_DETECT_RISING         (1<<0)
#define GPIO_EDGE_DETECT_FALLING        (1<<1)
#define GPIO_EDGE_DETECT_ASYNC_RISING   (1<<2)
#define GPIO_EDGE_DETECT_ASYNC_FALLING  (1<<3)

void gpio_select(const unsigned pin, const gpio_function_t mode);
void gpio_set(const unsigned pin, const int high);
unsigned char gpio_get(const unsigned pin);
void gpio_setpull(const unsigned pin, const gpio_pull_t pull);
void gpio_clear_irq(const unsigned pin);
void gpio_setedgedetect(const unsigned pin, const unsigned char edgedetect);

#endif /* GPIO_H__ */ 
