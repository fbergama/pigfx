//
// irq.c
// IRQ and exception handling
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2014-2020 Filippo Bergamasco, Christian Lehner

#include "irq.h"
#include "ee_printf.h"
#include "utils.h"
#include "exception.h"

typedef struct TAbortFrame
{
	unsigned int	sp_irq;
	unsigned int	lr_irq;
	unsigned int	r0;
	unsigned int	r1;
	unsigned int	r2;
	unsigned int	r3;
	unsigned int	r4;
	unsigned int	r5;
	unsigned int	r6;
	unsigned int	r7;
	unsigned int	r8;
	unsigned int	r9;
	unsigned int	r10;
	unsigned int	r11;
	unsigned int	r12;
	unsigned int	sp;
	unsigned int	lr;
	unsigned int	spsr;
	unsigned int	pc;
}
TAbortFrame;


rpi_irq_controller_t* pIRQController =  (rpi_irq_controller_t*)INTERRUPT_BASE;


IntHandler* (_irq_handlers[NBROFIRQ]) = {0};
void* (_irq_handlers_data[NBROFIRQ]) = {0};


void irq_attach_handler( unsigned int irq, IntHandler *phandler, void* pdata )
{
    if( irq < NBROFIRQ )
    {
        _irq_handlers[ irq ] = phandler;
        _irq_handlers_data[ irq ] = pdata;

        unsigned enableReg = irq / 32;
        unsigned bit = irq % 32;
        pIRQController->Enable_IRQs[enableReg] = (1 << bit);
    }
    enable_irq();
}

void __attribute__((interrupt("IRQ"))) irq_handler_(void)
{
    // Bit 17 on pending 2 means IRQ 49 is pending.
    if (pIRQController->IRQ_pending[1] & RPI_GPIO0_INTERRUPT_IRQ && _irq_handlers[49])
    {
        // IRQ 49
        IntHandler* hnd = _irq_handlers[49];
        hnd( _irq_handlers_data[49] );
    }
    // Bit 19 on basic means IRQ 57 is pending. This would actually be bit 25 in IRQ_pending_2
    else if( pIRQController->IRQ_pending[1] & RPI_UART_INTERRUPT_IRQ && _irq_handlers[57] )
    {
        // IRQ 57
        IntHandler* hnd = _irq_handlers[57];
        hnd( _irq_handlers_data[57] );

    }
    // Bit 11 in Basic means IRQ 9
    else if( pIRQController->IRQ_pending[0] & RPI_USB_IRQ && _irq_handlers[9] )
    {
        // IRQ 9
        IntHandler* hnd = _irq_handlers[9];
        hnd( _irq_handlers_data[9] );

    }
    else
    {
#if RPI<4
        ee_printf("ERROR: unhandled interrupt basic:%08x, pend1:%08x, pend2:%08x", pIRQController->IRQ_basic_pending, pIRQController->IRQ_pending[0], pIRQController->IRQ_pending[1]);
#else
        ee_printf("ERROR: unhandled interrupt pend0:%08x, pend1:%08x, pend2:%08x", pIRQController->IRQ_pending[0], pIRQController->IRQ_pending[1], pIRQController->IRQ_pending[2]);
#endif
        while (1)
        {
            // freeze
        }
    }

}

// Concept adapted from USPI
void exception_handler_(unsigned int nException, TAbortFrame *pFrame)
{
    char* excType;
    unsigned int FSR = 0, FAR = 0;
    if (nException == EXCEPTION_UNDEFINED_INSTRUCTION)excType = "Undefined instruction occurred!";
    else if (nException == EXCEPTION_PREFETCH_ABORT)
    {
        excType = "Prefetch abort exception occurred!";
		asm volatile ("mrc p15, 0, %0, c5, c0,  1" : "=r" (FSR));
		asm volatile ("mrc p15, 0, %0, c6, c0,  2" : "=r" (FAR));
    }
    else
    {
        excType = "Data abort exception occurred!";
		asm volatile ("mrc p15, 0, %0, c5, c0,  0" : "=r" (FSR));
		asm volatile ("mrc p15, 0, %0, c6, c0,  0" : "=r" (FAR));
    }

	unsigned int lr = pFrame->lr;
	unsigned int sp = pFrame->sp;

	if ((pFrame->spsr & 0x1F) == 0x12)	// IRQ mode?
	{
		lr = pFrame->lr_irq;
		sp = pFrame->sp_irq;
	}

    ee_printf("\e[2J");
    ee_printf("%s -> HALT\n", excType);

    ee_printf("PC %08x, FSR %08x, FAR %08x, SP %08x, LR %08x, PSR %08x\n", pFrame->pc, FSR, FAR, sp, lr, pFrame->spsr);

    while (1)
    {
        ;
    }
}
