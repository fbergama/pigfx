#include "buffered_uart.h"
#include "uart.h"
#include "irq.h"
#include "utils.h"
#include "nmalloc.h"

volatile unsigned int* UART0_DR;
volatile unsigned int* UART0_ITCR;
volatile unsigned int* UART0_IMSC;
volatile unsigned int* UART0_FR;

volatile char* uart_buffer;
volatile char* uart_buffer_start;
volatile char* uart_buffer_end;
volatile char* uart_buffer_limit;


void buart_fill_queue( __attribute__((unused)) void* data )
{
    while( !( *UART0_FR & 0x10)/*uart_poll()*/)
    {
        *uart_buffer_end++ = (char)( *UART0_DR & 0xFF /*uart_read_byte()*/);

        if( uart_buffer_end >= uart_buffer_limit )
           uart_buffer_end = uart_buffer;

        if( uart_buffer_end == uart_buffer_start )
        {
            uart_buffer_start++;
            if( uart_buffer_start >= uart_buffer_limit )
                uart_buffer_start = uart_buffer;
        }
    }

    /* Clear UART0 interrupts */
    *UART0_ITCR = 0xFFFFFFFF;
}


void buart_initialize_irq( unsigned int UART_BUFFER_SIZE )
{
    // UART buffer allocation
    uart_buffer = (volatile char*)nmalloc_malloc( UART_BUFFER_SIZE );
    uart_buffer_start = uart_buffer_end = uart_buffer;
    uart_buffer_limit = &( uart_buffer[ UART_BUFFER_SIZE ] );

    UART0_DR   = (volatile unsigned int*)0x20201000;
    UART0_IMSC = (volatile unsigned int*)0x20201038;
    UART0_ITCR = (volatile unsigned int*)0x20201044;
    UART0_FR   = (volatile unsigned int*)0x20201018;

    *UART0_IMSC = (1<<4) | (1<<7) | (1<<9); // Masked interrupts: RXIM + FEIM + BEIM (See pag 188 of BCM2835 datasheet)
    *UART0_ITCR = 0xFFFFFFFF; // Clear UART0 interrupts

    pIRQController->Enable_IRQs_2 = RPI_UART_INTERRUPT_IRQ;
    enable_irq();
    irq_attach_handler( 57, buart_fill_queue, 0 );
}


int buart_buffer_empty()
{
    return uart_buffer_start==uart_buffer_end;
}


char buart_next_char()
{
    char ch = *uart_buffer_start++;
    if( uart_buffer_start >= uart_buffer_limit )
        uart_buffer_start = uart_buffer;
    return ch;
}


void buart_purge()
{
    uart_buffer_start = uart_buffer_end;
}
