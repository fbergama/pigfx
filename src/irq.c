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
    // Bit 17 on pending 2 means IRQ 49 is pending.
    if (pIRQController->IRQ_pending_2 & RPI_GPIO0_INTERRUPT_IRQ && _irq_handlers[49])
    {
        // IRQ 49
        IntHandler* hnd = _irq_handlers[49];
        hnd( _irq_handlers_data[49] );
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
