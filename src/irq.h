//
// irq.h
// IRQ and exception handling
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2014-2020 Filippo Bergamasco, Christian Lehner

#ifndef _IRQ_H_
#define _IRQ_H_

#include "peri.h"

/* See http://www.valvers.com/open-software/raspberry-pi/step04-bare-metal-programming-in-c-pt4/ */

/** @brief See Section 7.5 of the BCM2835 ARM Peripherals documentation, the base
 *         address of the controller is actually xxxxB000, but there is a 0x200 offset
 *                 to the first addressable register for the interrupt controller, so offset the
 *                         base to the first register */


/** @brief Bits in the Enable_Basic_IRQs register to enable various interrupts.
 *         See the BCM2835 ARM Peripherals manual, section 7.5 */
#define RPI_BASIC_ARM_TIMER_IRQ         (1 << 0)
#define RPI_BASIC_ARM_MAILBOX_IRQ       (1 << 1)
#define RPI_BASIC_ARM_DOORBELL_0_IRQ    (1 << 2)
#define RPI_BASIC_ARM_DOORBELL_1_IRQ    (1 << 3)
#define RPI_BASIC_GPU_0_HALTED_IRQ      (1 << 4)
#define RPI_BASIC_GPU_1_HALTED_IRQ      (1 << 5)
#define RPI_BASIC_ACCESS_ERROR_1_IRQ    (1 << 6)
#define RPI_BASIC_ACCESS_ERROR_0_IRQ    (1 << 7)

#define RPI_AUX_INTERRUPT_IRQ           (1 << 29) /* 29 for IRQ register 1 means IRQ 29 in the table */
#define RPI_GPIO0_INTERRUPT_IRQ         (1 << 17) /* 17 for IRQ register 2 means IRQ 49 in the table */
#define RPI_GPIO1_INTERRUPT_IRQ         (1 << 18) /* 18 for IRQ register 2 means IRQ 50 in the table */
#define RPI_GPIO2_INTERRUPT_IRQ         (1 << 19) /* 19 for IRQ register 2 means IRQ 51 in the table */
#define RPI_GPIO3_INTERRUPT_IRQ         (1 << 20) /* 20 for IRQ register 2 means IRQ 52 in the table */
#define RPI_UART_INTERRUPT_IRQ          (1 << 25) /* 25 for IRQ register 2 means IRQ 57 in the table */
#define RPI_USB_IRQ                     (1 << 9)  /* 9 for IRQ register 0 means IRQ 9 in the table */
#define RPI_BASIC_ARM_TIMER_IRQ         (1 << 0)
#define RPI_SYSTEM_TIMER_3_IRQ          (1 << 3)

#if RPI<4
#define NBROFIRQ    64
#else
#define NBROFIRQ    80
#endif

#define IRQ_GPIO_0 49

#define MAX_GPIO_HANDLER    64

typedef void IntHandler( void *data );
void irq_attach_handler( unsigned int irq, IntHandler *phandler, void* pdata );
typedef void GpioHandler();
void fiq_attach_gpio_handler(unsigned int gpio, GpioHandler* gpiohandler);

#endif

