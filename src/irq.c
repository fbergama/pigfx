#include "irq.h"
#include "ee_printf.h"
#include "utils.h"


rpi_irq_controller_t* pIRQController =  (rpi_irq_controller_t*)INTERRUPT_BASE;


IntHandler* (_irq_handlers[63]) = {0};
void* (_irq_handlers_data[63]) = {0};


void irq_attach_handler( unsigned int irq, IntHandler *phandler, void* pdata )
{
    if( irq < 63 )
    {
        _irq_handlers[ irq ] = phandler;
        _irq_handlers_data[ irq ] = pdata;
    }
    enable_irq();
}


void __attribute__((interrupt("IRQ"))) irq_handler_(void)
{
    if( pIRQController->IRQ_pending_1 & (1<<29) &&
        _irq_handlers[29] )
    {
        // IRQ 29 AUX UART1
        IntHandler* hnd = _irq_handlers[29]; 
        hnd( _irq_handlers_data[29] );

    }
    // Bit 19 on basic means IRQ 57 is pending. This would actually be bit 25 in IRQ_pending_2
    else if( pIRQController->IRQ_basic_pending & (1<<19) &&
        _irq_handlers[57] )
    {
        // IRQ 57
        IntHandler* hnd = _irq_handlers[57]; 
        hnd( _irq_handlers_data[57] );

    }
    // Bit 11 in Basic means IRQ 9
    else if( pIRQController->IRQ_basic_pending & (1<<11) &&
        _irq_handlers[9] )
    {
        // IRQ 9
        IntHandler* hnd = _irq_handlers[9]; 
        hnd( _irq_handlers_data[9] );

    }

}
