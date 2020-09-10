//
// irq.c
// IRQ, FIQ and exception handling
//
// PiGFX is a bare metal kernel for the Raspberry Pi
// that implements a basic ANSI terminal emulator with
// the additional support of some primitive graphics functions.
// Copyright (C) 2014-2020 Filippo Bergamasco, Christian Lehner

#include "irq.h"
#include "ee_printf.h"
#include "utils.h"
#include "synchronize.h"
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

FiqHandler* _fiq_handler = 0;
IntHandler* (_irq_handlers[NBROFIRQ]) = {0};
void* (_irq_handlers_data[NBROFIRQ]) = {0};


void irq_attach_handler( unsigned int irq, IntHandler *phandler, void* pdata )
{
    if( irq < NBROFIRQ )
    {
        _irq_handlers[ irq ] = phandler;
        _irq_handlers_data[ irq ] = pdata;

        unsigned int enableReg = INTERRUPT_ENABLE_IRQs0 + irq / 32 * 4;
        unsigned int bit = irq % 32;
        W32(enableReg, (1 << bit));
    }
    EnableIRQs();
}

void fiq_attach_handler( unsigned int fiq, FiqHandler* phandler )
{
    if (fiq >= NBROFIRQ) return;
    _fiq_handler = phandler;
#if RPI<4
    W32(INTERRUPT_FIQ_CONTROL, fiq | (1<<7));   // Bit 7 is the enable bit
#else
    unsigned int enableReg = INTERRUPT_ENABLE_FIQs0 + fiq / 32 * 4;
    unsigned int bit = fiq % 32;
    W32(enableReg, (1 << bit));
#endif // RPI
    EnableFIQs();
}

void __attribute__((interrupt("IRQ"))) irq_handler_(void)
{
    // Bit 17 on pending 1 means IRQ 49 is pending.
    // IRQ 49 is the GPIO0..31 interrupt
    if ( R32(INTERRUPT_IRQ_PENDING_1) & RPI_GPIO0_INTERRUPT_IRQ && _irq_handlers[49])
    {
        // IRQ 49
        IntHandler* hnd = _irq_handlers[49];
        hnd( _irq_handlers_data[49] );
    }
    // Bit 25 on pending 1 means IRQ 57 is pending
    // IRQ 57 is the UART interrupt
    else if( R32(INTERRUPT_IRQ_PENDING_1) & RPI_UART_INTERRUPT_IRQ && _irq_handlers[57] )
    {
        // IRQ 57
        IntHandler* hnd = _irq_handlers[57];
        hnd( _irq_handlers_data[57] );

    }
    // Bit 9 in pending 0 means IRQ 9
    // IRQ 9 is the USB interrupt
    else if( R32(INTERRUPT_IRQ_PENDING_0) & RPI_USB_IRQ && _irq_handlers[9] )
    {
        // IRQ 9
        IntHandler* hnd = _irq_handlers[9];
        hnd( _irq_handlers_data[9] );

    }
    else
    {
        unsigned int lr;
        asm volatile ("MOV %0, LR\n" : "=r" (lr) );
#if RPI<4
        ee_printf("ERROR: unhandled interrupt basic:%08x, pend1:%08x, pend2:%08x, LR:%08x\n", R32(INTERRUPT_IRQ_BASIC_PENDING), R32(INTERRUPT_IRQ_PENDING_0), R32(INTERRUPT_IRQ_PENDING_1), lr);
#else
        ee_printf("ERROR: unhandled interrupt pend0:%08x, pend1:%08x, pend2:%08x, LR:%08x\n", R32(INTERRUPT_IRQ_PENDING_0), R32(INTERRUPT_IRQ_PENDING_1), R32(INTERRUPT_IRQ_PENDING_2), lr);
#endif
        while (1)
        {
            // freeze
        }
    }

}

void __attribute__((interrupt("FIQ"))) fiq_handler_(void)
{
    // only one FIQ possible, choose wisely
    if (_fiq_handler) _fiq_handler();
    else
    {
        ee_printf("ERROR: unhandled fast interrupt - > STOP\n");
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
