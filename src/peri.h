#ifndef PERI_H
#define PERI_H

// peri.h
// Peripheral Addresses of the different Raspberry Pi Models
// 15.05.2020 Christian Lehner

#if RPI==1
#define PERIPHERAL_BASE     0x20000000
#define BUSALIAS            0x40000000

#else
#if RPI==4
#define PERIPHERAL_BASE     0xFE000000
#define BUSALIAS            0xC0000000

#else       // RPI2&3
#define PERIPHERAL_BASE     0x3F000000
#define BUSALIAS            0xC0000000
#endif      //RPI==4

#endif      // RPI==1

// Base defined now

/////////////
// Mailbox //
/////////////
#define MBOX_BASE    (PERIPHERAL_BASE + 0xb880)  // 0x??00b880
#define MBOX_READ0   (MBOX_BASE + 0x00)
#define MBOX_PEEK0   (MBOX_BASE + 0x10)
#define MBOX_SENDER0 (MBOX_BASE + 0x14)
#define MBOX_STATUS0 (MBOX_BASE + 0x18)
#define MBOX_CONFIG0 (MBOX_BASE + 0x1c)
#define MBOX_WRITE1  (MBOX_BASE + 0x20)
#define MBOX_PEEK1   (MBOX_BASE + 0x30)
#define MBOX_SENDER1 (MBOX_BASE + 0x34)
#define MBOX_STATUS1 (MBOX_BASE + 0x38)
#define MBOX_CONFIG1 (MBOX_BASE + 0x3c)

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
// levels
#define GPIO_GPLEV0  (GPIO_BASE+0x34)     // GPIO Pin Level 0
#define GPIO_GPLEV1  (GPIO_BASE+0x38)     // GPIO Pin Level 1
// events
#define GPIO_GPEDS0  (GPIO_BASE+0x40)     // GPIO Pin Event Detect Status 0
#define GPIO_GPEDS1  (GPIO_BASE+0x44)     // GPIO Pin Event Detect Status 1
// edge detection
#define GPIO_GPREN0  (GPIO_BASE+0x4C)     // GPIO Pin Rising Edge Detect Enable 0
#define GPIO_GPREN1  (GPIO_BASE+0x50)     // GPIO Pin Rising Edge Detect Enable 1
#define GPIO_GPFEN0  (GPIO_BASE+0x58)     // GPIO Pin Falling Edge Detect Enable 0
#define GPIO_GPFEN1  (GPIO_BASE+0x5C)     // GPIO Pin Falling Edge Detect Enable 1
#define GPIO_GPAREN0 (GPIO_BASE+0x7C)     // GPIO Pin Async. Rising Edge Detect 0
#define GPIO_GPAREN1 (GPIO_BASE+0x80)     // GPIO Pin Async. Rising Edge Detect 1
#define GPIO_GPAFEN0 (GPIO_BASE+0x88)     // GPIO Pin Async. Falling Edge Detect 0
#define GPIO_GPAFEN1 (GPIO_BASE+0x8C)     // GPIO Pin Async. Falling Edge Detect 1
// Controls actuation of pull up/down to ALL GPIO pins.
#define GPIO_PUD     (GPIO_BASE+0x94) // 0x??200094
// Controls actuation of pull up/down for specific GPIO pin.
#define GPIO_PUDCLK0 (GPIO_BASE+0x98) // 0x??200098
#define GPIO_PUDCLK1 (GPIO_BASE+0x9C) // 0x??20009C
// New Pull Up / Pull Down Registers for the RPI4
#define GPIO_GPPINMUXSD	        (GPIO_BASE+0xD0) // 0x??2000D0
#define GPIO_PUP_PDN_CNTRL_REG0 (GPIO_BASE+0xE4) // 0x??2000E4
#define GPIO_PUP_PDN_CNTRL_REG1 (GPIO_BASE+0xE8) // 0x??2000E8
#define GPIO_PUP_PDN_CNTRL_REG2 (GPIO_BASE+0xEC) // 0x??2000EC
#define GPIO_PUP_PDN_CNTRL_REG3 (GPIO_BASE+0xF0) // 0x??2000F0

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

// DMA
#define DMA_BASE     (PERIPHERAL_BASE+0x7000)    // 0x??007000
#define DMA0_BASE    (DMA_BASE + 0x000)          // 0x??007000 DMA Channel 0 Register Set
#define DMA1_BASE    (DMA_BASE + 0x100)          // 0x??007100 DMA Channel 1 Register Set
#define DMA2_BASE    (DMA_BASE + 0x200)          // 0x??007200 DMA Channel 2 Register Set
#define DMA3_BASE    (DMA_BASE + 0x300)          // 0x??007300 DMA Channel 3 Register Set
#define DMA4_BASE    (DMA_BASE + 0x400)          // 0x??007400 DMA Channel 4 Register Set
#define DMA5_BASE    (DMA_BASE + 0x500)          // 0x??007500 DMA Channel 5 Register Set
#define DMA6_BASE    (DMA_BASE + 0x600)          // 0x??007600 DMA Channel 6 Register Set
#define DMA7_BASE    (DMA_BASE + 0x700)          // 0x??007700 DMA Channel 7 Register Set
#define DMA8_BASE    (DMA_BASE + 0x800)          // 0x??007800 DMA Channel 8 Register Set
#define DMA9_BASE    (DMA_BASE + 0x900)          // 0x??007900 DMA Channel 9 Register Set
#define DMA10_BASE   (DMA_BASE + 0xA00)          // 0x??007A00 DMA Channel 10 Register Set
#define DMA11_BASE   (DMA_BASE + 0xB00)          // 0x??007B00 DMA Channel 11 Register Set
#define DMA12_BASE   (DMA_BASE + 0xC00)          // 0x??007C00 DMA Channel 12 Register Set
#define DMA13_BASE   (DMA_BASE + 0xD00)          // 0x??007D00 DMA Channel 13 Register Set
#define DMA14_BASE   (DMA_BASE + 0xE00)          // 0x??007E00 DMA Channel 14 Register Set
#define DMA_INT_STATUS  (DMA_BASE + 0xFE0)       // 0x??007FE0 Interrupt Status of each DMA Channel
#define DMA_ENABLE   (DMA_BASE + 0xFF0)          // 0x??007FF0 Global Enable bits for each DMA Channel */
#define DMA15_BASE   (PERIPHERAL_BASE+0xE05000)  // 0x??E05000 DMA Channel 15 Register Set */

// Interrupt Controller
#define INTERRUPT_BASE                  (PERIPHERAL_BASE+0xB000)    // 0x??00B000

#if RPI<4
#define INTERRUPT_IRQ_BASIC_PENDING     (INTERRUPT_BASE+0x200)      // 0x??00B200
#define INTERRUPT_IRQ_PENDING_0         (INTERRUPT_BASE+0x204)      // 0x??00B204
#define INTERRUPT_IRQ_PENDING_1         (INTERRUPT_BASE+0x208)      // 0x??00B208
#define INTERRUPT_FIQ_CONTROL           (INTERRUPT_BASE+0x20C)      // 0x??00B20C
#define INTERRUPT_ENABLE_IRQs0          (INTERRUPT_BASE+0x210)      // 0x??00B210
#define INTERRUPT_ENABLE_IRQs1          (INTERRUPT_BASE+0x214)      // 0x??00B214
#define INTERRUPT_ENABLE_BASIC_IRQs     (INTERRUPT_BASE+0x218)      // 0x??00B218
#define INTERRUPT_DISABLE_IRQs0         (INTERRUPT_BASE+0x21C)      // 0x??00B21C
#define INTERRUPT_DISABLE_IRQs1         (INTERRUPT_BASE+0x220)      // 0x??00B220
#define INTERRUPT_DISABLE_BASIC_IRQs    (INTERRUPT_BASE+0x224)      // 0x??00B224
#else
#define INTERRUPT_IRQ_PENDING_0         (INTERRUPT_BASE+0x200)      // 0x??00B200
#define INTERRUPT_IRQ_PENDING_1         (INTERRUPT_BASE+0x204)      // 0x??00B204
#define INTERRUPT_IRQ_PENDING_2         (INTERRUPT_BASE+0x208)      // 0x??00B208
#define INTERRUPT_ENABLE_IRQs0          (INTERRUPT_BASE+0x210)      // 0x??00B210
#define INTERRUPT_ENABLE_IRQs1          (INTERRUPT_BASE+0x214)      // 0x??00B214
#define INTERRUPT_ENABLE_IRQs2          (INTERRUPT_BASE+0x218)      // 0x??00B218
#define INTERRUPT_DISABLE_IRQs0         (INTERRUPT_BASE+0x220)      // 0x??00B220
#define INTERRUPT_DISABLE_IRQs1         (INTERRUPT_BASE+0x224)      // 0x??00B224
#define INTERRUPT_DISABLE_IRQs2         (INTERRUPT_BASE+0x228)      // 0x??00B228
#define INTERRUPT_FIQ_PENDING_0         (INTERRUPT_BASE+0x300)      // 0x??00B300
#define INTERRUPT_FIQ_PENDING_1         (INTERRUPT_BASE+0x304)      // 0x??00B304
#define INTERRUPT_FIQ_PENDING_2         (INTERRUPT_BASE+0x308)      // 0x??00B308
#define INTERRUPT_ENABLE_FIQs0          (INTERRUPT_BASE+0x310)      // 0x??00B310
#define INTERRUPT_ENABLE_FIQs1          (INTERRUPT_BASE+0x314)      // 0x??00B314
#define INTERRUPT_ENABLE_FIQs2          (INTERRUPT_BASE+0x318)      // 0x??00B318
#define INTERRUPT_DISABLE_FIQs0         (INTERRUPT_BASE+0x320)      // 0x??00B320
#define INTERRUPT_DISABLE_FIQs1         (INTERRUPT_BASE+0x324)      // 0x??00B324
#define INTERRUPT_DISABLE_FIQs2         (INTERRUPT_BASE+0x328)      // 0x??00B328
#endif // RPI

//
// External Mass Media Controller (SD Card)
//
#define EMMC_BASE		(PERIPHERAL_BASE+0x300000)   // 0x??300000
#define EMMC2_BASE		(PERIPHERAL_BASE+0x340000)   // 0x??340000

#endif      //PERI_H
