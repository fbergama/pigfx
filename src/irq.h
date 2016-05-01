#ifndef _IRQ_H_
#define _IRQ_H_


/* See http://www.valvers.com/open-software/raspberry-pi/step04-bare-metal-programming-in-c-pt4/ */

/** @brief See Section 7.5 of the BCM2835 ARM Peripherals documentation, the base
 *         address of the controller is actually xxxxB000, but there is a 0x200 offset
 *                 to the first addressable register for the interrupt controller, so offset the
 *                         base to the first register */
#define RPI_INTERRUPT_CONTROLLER_BASE   ( 0x2000B200 )

/** @brief The interrupt controller memory mapped register set */
typedef struct {
    volatile unsigned int IRQ_basic_pending;
    volatile unsigned int IRQ_pending_1;
    volatile unsigned int IRQ_pending_2;
    volatile unsigned int FIQ_control;
    volatile unsigned int Enable_IRQs_1;
    volatile unsigned int Enable_IRQs_2;
    volatile unsigned int Enable_Base_IRQs;
    volatile unsigned int Disable_IRQs_1;
    volatile unsigned int Disable_IRQs_2;
    volatile unsigned int Disable_Base_IRQs;
} rpi_irq_controller_t;

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

#define RPI_UART_INTERRUPT_IRQ          (1 << 25) /* 25 for IRQ register 2 means IRQ 57 in the table */

/** @brief The BCM2835 Interupt controller peripheral at it's base address */
extern rpi_irq_controller_t* pIRQController;

#endif

