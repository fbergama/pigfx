#ifndef PERI_H
#define PERI_H

// peri.h
// Peripheral Addresses of the different Raspberry Pi Models
// 15.05.2020 Christian Lehner

#if RPI==1
#define PERIPHERAL_BASE     0x20000000

#else
#if RPI==4
#define PERIPHERAL_BASE     0xFE000000

#else       // RPI2&3
#define PERIPHERAL_BASE     0x3F000000
#endif      //RPI==4

#endif      // RPI==1

// Base defined now

///////////////////////////
// Define GPIO addresses //
///////////////////////////
#define GPIO_BASE    (PERIPHERAL_BASE + 0x200000)   // 0x??200000
// function selector
#define GPIO_FSEL0   (GPIO_BASE+0x00) // 0x??200000
#define GPIO_FSEL1   (GPIO_BASE+0x04) // 0x??200004
#define GPIO_FSEL2   (GPIO_BASE+0x08) // 0x??200008
#define GPIO_FSEL3   (GPIO_BASE+0x0C) // 0x??20000C
#define GPIO_FSEL4   (GPIO_BASE+0x10) // 0x??200010
#define GPIO_FSEL5   (GPIO_BASE+0x14) // 0x??200014
// set and clear pin output
#define GPIO_SET0    (GPIO_BASE+0x1C) // 0x??20001C
#define GPIO_SET1    (GPIO_BASE+0x20) // 0x??200020
#define GPIO_CLR0    (GPIO_BASE+0x28) // 0x??200028
#define GPIO_CLR1    (GPIO_BASE+0x2C) // 0x??20002C
// Controls actuation of pull up/down to ALL GPIO pins.
#define GPIO_PUD     (GPIO_BASE+0x94) // 0x??200094
// Controls actuation of pull up/down for specific GPIO pin.
#define GPIO_PUDCLK0 (GPIO_BASE+0x98) // 0x??200098
#define GPIO_PUDCLK1 (GPIO_BASE+0x9C) // 0x??20009C

///////////////////////////
// Define UART Addresses //
///////////////////////////
// The base address for UART.
#define UART0_BASE   (GPIO_BASE + 0x1000) // for raspi4 0xFE201000, raspi2 & 3 0x3F201000, and 0x20201000 for raspi1
#define UART0_DR     (UART0_BASE + 0x00)  // 0x??201000
#define UART0_RSRECR (UART0_BASE + 0x04)  // 0x??201004
#define UART0_FR     (UART0_BASE + 0x18)  // 0x??201018
#define UART0_ILPR   (UART0_BASE + 0x20)  // 0x??201020
#define UART0_IBRD   (UART0_BASE + 0x24)  // 0x??201024
#define UART0_FBRD   (UART0_BASE + 0x28)  // 0x??201028
#define UART0_LCRH   (UART0_BASE + 0x2C)  // 0x??20102C
#define UART0_CR     (UART0_BASE + 0x30)  // 0x??201030
#define UART0_IFLS   (UART0_BASE + 0x34)  // 0x??201034
#define UART0_IMSC   (UART0_BASE + 0x38)  // 0x??201038
#define UART0_RIS    (UART0_BASE + 0x3C)  // 0x??20103C
#define UART0_MIS    (UART0_BASE + 0x40)  // 0x??201040
#define UART0_ICR    (UART0_BASE + 0x44)  // 0x??201044
#define UART0_DMACR  (UART0_BASE + 0x48)  // 0x??201048
#define UART0_ITCR   (UART0_BASE + 0x80)  // 0x??201080
#define UART0_ITIP   (UART0_BASE + 0x84)  // 0x??201084
#define UART0_ITOP   (UART0_BASE + 0x88)  // 0x??201088
#define UART0_TDR    (UART0_BASE + 0x8C)  // 0x??20108C

// UART1
#define AUX_BASE        (GPIO_BASE+0x15000) // for raspi4 0xFE215000, raspi2 & 3 0x3F215000, and 0x20215000 for raspi1
#define AUX_IRQ         (AUX_BASE+0x00)
#define AUX_ENABLES     (AUX_BASE+0x04)
#define AUX_MU_IO_REG   (AUX_BASE+0x40)
#define AUX_MU_IER_REG  (AUX_BASE+0x44)
#define AUX_MU_IIR_REG  (AUX_BASE+0x48)
#define AUX_MU_LCR_REG  (AUX_BASE+0x4C)
#define AUX_MU_MCR_REG  (AUX_BASE+0x50)
#define AUX_MU_LSR_REG  (AUX_BASE+0x54)
#define AUX_MU_MSR_REG  (AUX_BASE+0x58)
#define AUX_MU_SCRATCH  (AUX_BASE+0x5C)
#define AUX_MU_CNTL_REG (AUX_BASE+0x60)
#define AUX_MU_STAT_REG (AUX_BASE+0x64)
#define AUX_MU_BAUD_REG (AUX_BASE+0x68)

///////////
// Timer //
///////////
#define TIMER_BASE   (PERIPHERAL_BASE+0x3000)    // 0x??003000
#define TIMER_CS     (TIMER_BASE+0x00)           // 0x??003000   System Timer Control and Status
#define TIMER_CLO    (TIMER_BASE+0x04)           // 0x??003004   System Timer Counter 1MHz Lower 32 bits
#define TIMER_CHI    (TIMER_BASE+0x08)           // 0x??003008   System Timer Counter 1MHz Upper 32 bits
#define TIMER_C0     (TIMER_BASE+0x0C)           // 0x??00300C   System Timer Compare 0; corresponds to IRQ line 0.
#define TIMER_C1     (TIMER_BASE+0x10)           // 0x??003010   System Timer Compare 1; corresponds to IRQ line 1.
#define TIMER_C2     (TIMER_BASE+0x14)           // 0x??003014   System Timer Compare 2; corresponds to IRQ line 2.
#define TIMER_C3     (TIMER_BASE+0x18)           // 0x??003018   System Timer Compare 3; corresponds to IRQ line 3.

/////////////
// Various //
/////////////
// The offsets for Mailbox registers
#define MBOX_BASE    (PERIPHERAL_BASE + 0xB880)  // 0x??00B880
#define MBOX_READ    (MBOX_BASE + 0x00)          // 0x??00B880
#define MBOX_STATUS  (MBOX_BASE + 0x18)          // 0x??00B898
#define MBOX_WRITE   (MBOX_BASE + 0x20)          // 0x??00B8A0

// DMA
#define DMA_BASE     (PERIPHERAL_BASE+0x7000)    // 0x??007000

// Interrupt Controller
#define INTERRUPT_BASE     (PERIPHERAL_BASE+0xB200)    // 0x??00B200

#endif      //PERI_H
